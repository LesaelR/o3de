/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#ifndef __IMGUI_MANAGER_H__
#define __IMGUI_MANAGER_H__

#pragma once

#ifdef IMGUI_ENABLED

#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Input/Events/InputTextEventListener.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <ISystem.h>
#include <imgui/imgui.h>
#include <ImGuiBus.h>
#include <VertexFormats.h>

#if defined(LOAD_IMGUI_LIB_DYNAMICALLY)  && !defined(AZ_MONOLITHIC_BUILD)
#include <AzCore/Module/DynamicModuleHandle.h>
#endif // defined(LOAD_IMGUI_LIB_DYNAMICALLY)  && !defined(AZ_MONOLITHIC_BUILD)

namespace ImGui
{
    class ImGuiManager
        : public AzFramework::InputChannelEventListener
        , public AzFramework::InputTextEventListener
        , public ImGuiManagerBus::Handler
        , public AzFramework::WindowNotificationBus::Handler
    {
    public:
        void Initialize();
        void Shutdown();

        // This is called by the ImGuiGem to Register CVARs at the correct time
        void RegisterImGuiCVARs();
    protected:
        void RenderImGuiBuffers(const ImVec2& scaleRects);

        // -- ImGuiManagerBus Interface -------------------------------------------------------------------
        DisplayState GetEditorWindowState() const override { return m_editorWindowState; }
        void SetEditorWindowState(DisplayState state) override { m_editorWindowState = state; }
        DisplayState GetClientMenuBarState() const override { return m_clientMenuBarState; }
        void SetClientMenuBarState(DisplayState state) override { m_clientMenuBarState = state; }
        bool IsControllerSupportModeEnabled(ImGuiControllerModeFlags::FlagType controllerMode) const override;
        void EnableControllerSupportMode(ImGuiControllerModeFlags::FlagType controllerMode, bool enable) override;
        void SetControllerMouseSensitivity(float sensitivity) { m_controllerMouseSensitivity = sensitivity; }
        float GetControllerMouseSensitivity() const { return m_controllerMouseSensitivity; }
        bool GetEnableDiscreteInputMode() const override { return m_enableDiscreteInputMode; }
        void SetEnableDiscreteInputMode(bool enabled) override { m_enableDiscreteInputMode = enabled; }
        ImGuiResolutionMode GetResolutionMode() const override { return m_resolutionMode; }
        void SetResolutionMode(ImGuiResolutionMode mode) override { m_resolutionMode = mode; }
        const ImVec2& GetImGuiRenderResolution() const override { return m_renderResolution; }
        void SetImGuiRenderResolution(const ImVec2& res) override { m_renderResolution = res; }
        void OverrideRenderWindowSize(uint32_t width, uint32_t height) override;
        void RestoreRenderWindowSizeToDefault() override;
        void SetDpiScalingFactor(float dpiScalingFactor) override;
        float GetDpiScalingFactor() const override;
        void Render() override;
        void ToggleThroughImGuiVisibleState() override;
        // -- ImGuiManagerBus Interface -------------------------------------------------------------------

        // -- AzFramework::InputChannelEventListener and AzFramework::InputTextEventListener Interface ------------
        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;
        bool OnInputTextEventFiltered(const AZStd::string& textUTF8) override;
        int GetPriority() const override { return AzFramework::InputChannelEventListener::GetPriorityDebugUI(); }
        // -- AzFramework::InputChannelEventListener and AzFramework::InputTextEventListener Interface ------------

        // AzFramework::WindowNotificationBus::Handler overrides...
        void OnWindowResized(uint32_t width, uint32_t height) override;

        // Sets up initial window size and listens for changes
        void InitWindowSize();

        // A function to toggle through the available ImGui Visibility States
        void ToggleThroughImGuiVisibleState(int controllerIndex);

    private:
        ImGuiContext* m_imguiContext = nullptr;
        DisplayState m_clientMenuBarState = DisplayState::Hidden;
        DisplayState m_editorWindowState = DisplayState::Hidden;

        // ImGui Resolution Settings
        ImGuiResolutionMode m_resolutionMode = ImGuiResolutionMode::MatchRenderResolution;
        ImVec2 m_renderResolution = ImVec2(1920.0f, 1080.0f);
        ImVec2 m_lastRenderResolution;
        AzFramework::WindowSize m_windowSize = AzFramework::WindowSize(1920, 1080);
        bool m_overridingWindowSize = false;

        // Rendering buffers
        std::vector<SVF_P3F_C4B_T2F> m_vertBuffer;
        std::vector<uint16> m_idxBuffer;

        //Controller navigation
        static const int MaxControllerNumber = 4;
        int m_currentControllerIndex;
        bool m_button1Pressed, m_button2Pressed, m_menuBarStatusChanged;

        bool m_hardwardeMouseConnected = false;
        bool m_enableDiscreteInputMode = false;
        ImGuiControllerModeFlags::FlagType m_controllerModeFlags = 0;
        float m_controllerMouseSensitivity = 4.0f;
        float m_controllerMousePosition[2] = { 0.0f, 0.0f };
        float m_lastPrimaryTouchPosition[2] = { 0.0f, 0.0f };
        bool m_useLastPrimaryTouchPosition = false;
        bool m_simulateBackspaceKeyPressed = false;

#if defined(LOAD_IMGUI_LIB_DYNAMICALLY)  && !defined(AZ_MONOLITHIC_BUILD)
        AZStd::unique_ptr<AZ::DynamicModuleHandle>  m_imgSharedLib;
#endif // defined(LOAD_IMGUI_LIB_DYNAMICALLY)  && !defined(AZ_MONOLITHIC_BUILD)
    };

    void AddMenuItemHelper(bool& control, const char* show, const char* hide);
}

#endif // ~IMGUI_ENABLED
#endif //__IMGUI_MANAGER_H__
