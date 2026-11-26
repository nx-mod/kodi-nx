/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinSystemSwitchGLESContext.h"

#include "platform/switch/Shutdown.h"
#include "ServiceBroker.h"
#include "application/AppInboundProtocol.h"
#include "cores/RetroPlayer/process/RPProcessInfo.h"
#include "cores/RetroPlayer/rendering/VideoRenderers/RPRendererOpenGLES.h"
#include "cores/VideoPlayer/DVDCodecs/DVDFactoryCodec.h"
#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGLES.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "rendering/gles/ScreenshotSurfaceGLES.h"
#include "guilib/DispResource.h"
#include "input/keyboard/XBMC_keysym.h"
#include "settings/DisplaySettings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "windowing/GraphicContext.h"
#include "windowing/Resolution.h"
#include "windowing/WindowSystemFactory.h"
#include "windowing/XBMC_events.h"

#include <algorithm>
#include <iterator>
#include <cstring>

using namespace KODI::WINDOWING::SWITCH;

namespace
{
struct PadKey
{
  uint64_t button;
  uint32_t sym;
};

// Kodi's default keyboard keymap gives these keys their usual meaning.
// A = select, B = back, X = context menu, Y = info, +/- = menu/OSD,
// L/R = page/skip, ZL/ZR = rewind/fast-forward, sticks double the d-pad.
constexpr PadKey kPadKeys[] = {
    {HidNpadButton_Up | HidNpadButton_StickLUp, XBMCK_UP},
    {HidNpadButton_Down | HidNpadButton_StickLDown, XBMCK_DOWN},
    {HidNpadButton_Left | HidNpadButton_StickLLeft, XBMCK_LEFT},
    {HidNpadButton_Right | HidNpadButton_StickLRight, XBMCK_RIGHT},
    {HidNpadButton_A, XBMCK_RETURN},
    {HidNpadButton_B, XBMCK_BACKSPACE},
    {HidNpadButton_X, XBMCK_c},
    {HidNpadButton_Y, XBMCK_i},
    {HidNpadButton_Plus, XBMCK_m},
    {HidNpadButton_Minus, XBMCK_TAB},
    {HidNpadButton_L, XBMCK_PAGEUP},
    {HidNpadButton_R, XBMCK_PAGEDOWN},
    {HidNpadButton_ZL, XBMCK_r},
    {HidNpadButton_ZR, XBMCK_f},
    {HidNpadButton_StickL, XBMCK_SPACE},
    {HidNpadButton_StickR, XBMCK_s},
    {HidNpadButton_StickRUp, XBMCK_PLUS},
    {HidNpadButton_StickRDown, XBMCK_MINUS},
};

static_assert(std::size(kPadKeys) == 18, "m_repeatAtNs in the header must have one slot per kPadKeys entry");

// RetroPlayer needs a registered process control; the Switch 2 has no platform specifics.
class CRPProcessInfoSwitch : public KODI::RETRO::CRPProcessInfo
{
public:
  CRPProcessInfoSwitch() : CRPProcessInfo("Switch") {}
  static std::unique_ptr<KODI::RETRO::CRPProcessInfo> Create()
  {
    return std::make_unique<CRPProcessInfoSwitch>();
  }
};

bool SendEvent(XBMC_Event& event)
{
  std::shared_ptr<CAppInboundProtocol> appPort = CServiceBroker::GetAppPort();
  return appPort ? appPort->OnEvent(event) : false;
}
} // namespace

void CWinSystemSwitchGLESContext::Register()
{
  CWindowSystemFactory::RegisterWindowSystem(CreateWinSystem, "switch");
}

std::unique_ptr<CWinSystemBase> CWinSystemSwitchGLESContext::CreateWinSystem()
{
  return std::make_unique<CWinSystemSwitchGLESContext>();
}

CWinSystemSwitchGLESContext::CWinSystemSwitchGLESContext() = default;

CWinSystemSwitchGLESContext::~CWinSystemSwitchGLESContext()
{
  DestroyWindowSystem();
}

bool CWinSystemSwitchGLESContext::InitWindowSystem()
{
  // Per-windowing registrations, as in the gbm/X11 GLES contexts: without a video renderer,
  // VideoPlayer loops on "OutputPicture - failed to configure renderer" (rev 10 flight).
  // VideoPlayer keeps the default CProcessInfo; decoding is FFmpeg software (no HW accels).
  VIDEOPLAYER::CRendererFactory::ClearRenderer();
  CDVDFactoryCodec::ClearHWAccels();
  CLinuxRendererGLES::Register();
  KODI::RETRO::CRPProcessInfo::RegisterProcessControl(CRPProcessInfoSwitch::Create);
  KODI::RETRO::CRPProcessInfo::RegisterRendererFactory(new KODI::RETRO::CRendererFactoryOpenGLES);
  CScreenshotSurfaceGLES::Register();

  padConfigureInput(1, HidNpadStyleSet_NpadStandard);
  padInitializeDefault(&m_pad);
  hidInitializeTouchScreen();

  m_window = nwindowGetDefault();

  s32 w = 0, h = 0;
  if (R_SUCCEEDED(appletGetDefaultDisplayResolution(&w, &h)) && w > 0 && h > 0)
  {
    m_displayWidth = std::min<s32>(w, 1920);
    m_displayHeight = std::min<s32>(h, 1080);
  }
  CLog::Log(LOGINFO, "Switch: display {}x{}", m_displayWidth, m_displayHeight);

  if (!m_eglContext.CreateDisplay(EGL_DEFAULT_DISPLAY))
    return false;
  if (!m_eglContext.InitializeDisplay(EGL_OPENGL_ES_API))
    return false;
  if (!m_eglContext.ChooseConfig(EGL_OPENGL_ES3_BIT))
    return false;
  // CEGLContextUtils::ChooseConfig returns true even when no config matched; fail loudly instead.
  if (m_eglContext.GetEGLConfig() == nullptr)
  {
    CLog::Log(LOGFATAL, "Switch: no EGL config matched (ES3, RGB888, window)");
    return false;
  }

  CEGLAttributesVec contextAttribs;
  contextAttribs.Add({{EGL_CONTEXT_MAJOR_VERSION, 3}, {EGL_CONTEXT_MINOR_VERSION, 0}});
  if (!m_eglContext.CreateContext(contextAttribs))
    return false;

  return CWinSystemBase::InitWindowSystem();
}

bool CWinSystemSwitchGLESContext::DestroyWindowSystem()
{
  m_eglContext.Destroy();
  return true;
}

bool CWinSystemSwitchGLESContext::CreateNewWindow(const std::string& name,
                                                   bool fullScreen,
                                                   RESOLUTION_INFO& res)
{
  if (m_bWindowCreated)
  {
    // The NWindow has one fixed size; nothing to recreate.
    return true;
  }

  nwindowSetDimensions(m_window, m_displayWidth, m_displayHeight);
  if (!m_eglContext.CreateSurface(reinterpret_cast<EGLNativeWindowType>(m_window)))
    return false;
  if (!m_eglContext.BindContext())
    return false;

  m_nWidth = m_displayWidth;
  m_nHeight = m_displayHeight;
  m_bFullScreen = true;
  m_bWindowCreated = true;
  return true;
}

bool CWinSystemSwitchGLESContext::DestroyWindow()
{
  m_eglContext.DestroySurface();
  m_bWindowCreated = false;
  return true;
}

bool CWinSystemSwitchGLESContext::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  CRenderSystemGLES::ResetRenderSystem(m_displayWidth, m_displayHeight);
  return true;
}

bool CWinSystemSwitchGLESContext::SetFullScreen(bool fullScreen,
                                                 RESOLUTION_INFO& res,
                                                 bool blankOtherDisplays)
{
  CreateNewWindow("", true, res);
  CRenderSystemGLES::ResetRenderSystem(m_displayWidth, m_displayHeight);
  return true;
}

void CWinSystemSwitchGLESContext::UpdateResolutions()
{
  CWinSystemBase::UpdateResolutions();

  RESOLUTION_INFO& desktop = CDisplaySettings::GetInstance().GetResolutionInfo(RES_DESKTOP);
  UpdateDesktopResolution(desktop, "Switch", m_displayWidth, m_displayHeight, 60.0f, 0);
  CServiceBroker::GetWinSystem()->GetGfxContext().ResetOverscan(desktop);
  CDisplaySettings::GetInstance().ClearCustomResolutions();
  CDisplaySettings::GetInstance().AddResolutionInfo(desktop);
  CDisplaySettings::GetInstance().ApplyCalibrations();
}

void CWinSystemSwitchGLESContext::Register(IDispResource* resource)
{
  std::unique_lock<CCriticalSection> lock(m_resourceSection);
  m_resources.push_back(resource);
}

void CWinSystemSwitchGLESContext::Unregister(IDispResource* resource)
{
  std::unique_lock<CCriticalSection> lock(m_resourceSection);
  m_resources.erase(std::remove(m_resources.begin(), m_resources.end(), resource),
                    m_resources.end());
}

void CWinSystemSwitchGLESContext::SetVSyncImpl(bool enable)
{
  m_eglContext.SetVSync(enable);
}

void CWinSystemSwitchGLESContext::PresentRenderImpl(bool rendered)
{
  if (rendered && !m_eglContext.TrySwapBuffers())
    CEGLUtils::Log(LOGERROR, "eglSwapBuffers failed");
}

void CWinSystemSwitchGLESContext::PushKey(uint32_t sym, bool down)
{
  XBMC_Event event{};
  event.type = down ? XBMC_KEYDOWN : XBMC_KEYUP;
  event.key.keysym.sym = static_cast<XBMCKey>(sym);
  event.key.keysym.scancode = 0;
  event.key.keysym.mod = XBMCKMOD_NONE;
  event.key.keysym.unicode = 0;
  SendEvent(event);
}

void CWinSystemSwitchGLESContext::PollPad()
{
  padUpdate(&m_pad);
  const uint64_t held = padGetButtons(&m_pad);

  // Kodi expects the window system to auto-repeat held keys (scrolling, long press).
  constexpr uint64_t kRepeatDelayNs = 400000000ULL;
  constexpr uint64_t kRepeatRateNs = 80000000ULL;
  const uint64_t nowNs = armTicksToNs(armGetSystemTick());
  for (size_t i = 0; i < std::size(kPadKeys); ++i)
  {
    const PadKey& key = kPadKeys[i];
    const bool down = (held & key.button) != 0;
    const bool before = (m_heldMapped & key.button) != 0;
    if (down != before)
    {
      PushKey(key.sym, down);
      m_repeatAtNs[i] = nowNs + kRepeatDelayNs;
    }
    else if (down && nowNs >= m_repeatAtNs[i])
    {
      PushKey(key.sym, true);
      m_repeatAtNs[i] = nowNs + kRepeatRateNs;
    }
  }
  m_heldMapped = held;
}

void CWinSystemSwitchGLESContext::PollTouch()
{
  HidTouchScreenState state{};
  if (hidGetTouchScreenStates(&state, 1) == 0)
    return;

  // The touch panel reports 1280x720 coordinates in handheld mode.
  const float sx = static_cast<float>(m_displayWidth) / 1280.f;
  const float sy = static_cast<float>(m_displayHeight) / 720.f;

  if (state.count > 0)
  {
    const float x = state.touches[0].x * sx;
    const float y = state.touches[0].y * sy;
    XBMC_Event motion{};
    motion.type = XBMC_MOUSEMOTION;
    motion.motion.x = static_cast<uint16_t>(x);
    motion.motion.y = static_cast<uint16_t>(y);
    SendEvent(motion);
    if (!m_touchDown)
    {
      XBMC_Event press{};
      press.type = XBMC_MOUSEBUTTONDOWN;
      press.button.button = 1;
      press.button.x = static_cast<uint16_t>(x);
      press.button.y = static_cast<uint16_t>(y);
      SendEvent(press);
    }
    m_touchDown = true;
    m_touchX = x;
    m_touchY = y;
  }
  else if (m_touchDown)
  {
    XBMC_Event release{};
    release.type = XBMC_MOUSEBUTTONUP;
    release.button.button = 1;
    release.button.x = static_cast<uint16_t>(m_touchX);
    release.button.y = static_cast<uint16_t>(m_touchY);
    SendEvent(release);
    m_touchDown = false;
  }
}

void CWinSystemSwitchGLESContext::HandleApplet()
{
  // appletMainLoop() processes HOME/sleep messages and returns false on an exit request.
  if (!appletMainLoop())
  {
    g_switchShuttingDown = true;
    FILE* mark = fopen("sdmc:/switch/kodi/boot.txt", "a");
    if (mark)
    {
      fprintf(mark, "applet-exit-requested\n");
      fclose(mark);
    }
    XBMC_Event quit{};
    quit.type = XBMC_QUIT;
    SendEvent(quit);
  }
}

std::atomic<bool> g_switchShuttingDown{false};

bool CWinSystemSwitchGLESContext::MessagePump()
{
  if (g_switchShuttingDown)
    return false;
  HandleApplet();
  PollPad();
  PollTouch();
  return true;
}
