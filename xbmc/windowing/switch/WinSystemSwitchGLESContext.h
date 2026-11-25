/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "rendering/gles/RenderSystemGLES.h"
#include "threads/CriticalSection.h"
#include "utils/EGLUtils.h"
#include "windowing/WinSystem.h"

#include <memory>
#include <vector>

#include <switch.h>

class IDispResource;

namespace KODI
{
namespace WINDOWING
{
namespace SWITCH
{

// One full-screen window on the default NWindow, GLES 3 through Mesa EGL.
// Input: the main controller is mapped to keyboard keys, the touch screen to the mouse.
class CWinSystemSwitchGLESContext : public CWinSystemBase, public CRenderSystemGLES
{
public:
  CWinSystemSwitchGLESContext();
  ~CWinSystemSwitchGLESContext() override;

  static void Register();
  static std::unique_ptr<CWinSystemBase> CreateWinSystem();

  const std::string GetName() override { return "switch"; }
  CRenderSystemBase* GetRenderSystem() override { return this; }

  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;
  bool CreateNewWindow(const std::string& name, bool fullScreen, RESOLUTION_INFO& res) override;
  bool DestroyWindow() override;
  bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) override;
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;
  void UpdateResolutions() override;
  bool CanDoWindowed() override { return false; }
  bool SupportsScreenMove() override { return false; }
  bool HasCursor() override { return false; }
  void Register(IDispResource* resource) override;
  void Unregister(IDispResource* resource) override;
  bool MessagePump() override;
  void* GetHWContext() override { return m_eglContext.GetEGLContext(); }

  EGLDisplay GetEGLDisplay() const { return m_eglContext.GetEGLDisplay(); }

protected:
  void SetVSyncImpl(bool enable) override;
  void PresentRenderImpl(bool rendered) override;

private:
  void PollPad();
  void PollTouch();
  void PushKey(uint32_t sym, bool down);
  void HandleApplet();

  CEGLContextUtils m_eglContext;
  NWindow* m_window = nullptr;
  int m_displayWidth = 1920;
  int m_displayHeight = 1080;

  PadState m_pad{};
  uint64_t m_heldMapped = 0;
  uint64_t m_repeatAtNs[18] = {}; // per kPadKeys entry: next auto-repeat time while held
  bool m_touchDown = false;
  float m_touchX = 0.f;
  float m_touchY = 0.f;

  CCriticalSection m_resourceSection;
  std::vector<IDispResource*> m_resources;
};

} // namespace SWITCH
} // namespace WINDOWING
} // namespace KODI
