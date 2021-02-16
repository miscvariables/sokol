#pragma once
#include<stdio.h>
#include<math.h>
#include "imgui.h"
#include <android/log.h>
namespace ImGui {

#define SIGN(x) (((x) > 0) - ((x) < 0))

#ifdef IMGUI_TOUCH_IMPL
typedef enum TOUCH_EVENT_TYPE {
    TOUCH_END,
    TOUCH_LEAVE, //==LEAVE
    TOUCH_BEGIN,
    TOUCH_ENTER, //==BEGIN
    TOUCH_MOVE
} TOUCH_EVENT_TYPE;
typedef struct TouchState {
    ImVec2 Velocity;
    ImVec2 PosPrev;
    ImVec2 PosCurr;
    ImVec2 ScrollLast;
    double EventTime;
    TOUCH_EVENT_TYPE EventType;
    struct TouchStateConfig {
        float jitter;
        float Valpha;
        float Abeta;
    } cfg;
    void reset() {
        Velocity.x = Velocity.y = 0.0f;
        PosPrev.x = PosPrev.y = 0.0f;
        PosCurr.x = PosCurr.y = 0.0f;
        EventTime = 0.0;
        EventType = TOUCH_END; //An empty state is touch ended 2021/02/14
    }
    TouchState() { 
        reset();
        cfg.jitter = 2.2f;
        cfg.Valpha = 0.5f;
        cfg.Abeta  = 0.04f;
    }
} TouchState;
    
TouchState GTouchState;

#define IMGUI_TOUCH_IMPL
void TouchScrollStart() {
}
void TouchScrollConfig() {
}
void TouchScrollEvent(TOUCH_EVENT_TYPE type, float x, float y) {
    GTouchState.EventTime = ImGui::GetTime();
    switch(type) {
        case TOUCH_END:
        case TOUCH_LEAVE:
            GTouchState.PosCurr.x = x;
            GTouchState.PosCurr.y = y;
            GTouchState.EventType = TOUCH_END;
__android_log_print(ANDROID_LOG_INFO, "TOUCH", "END=[%.1f, %.1f] v=%.1f\n", 
x, y, GTouchState.Velocity.y);
            break;
        case TOUCH_BEGIN:
        case TOUCH_ENTER:
            //GTouchState.Velocity.x = x - GTouchState.PosPrev.x;
            //GTouchState.Velocity.y = y - GTouchState.PosPrev.y;
            GTouchState.PosPrev.x = x;
            GTouchState.PosPrev.y = y;
            GTouchState.PosCurr.x = x;
            GTouchState.PosCurr.y = y;
            GTouchState.Velocity.x = 0;
            GTouchState.Velocity.y = 0;
            GTouchState.ScrollLast.x = -1;
            GTouchState.ScrollLast.y = -1;
            GTouchState.EventType = TOUCH_BEGIN;
//__android_log_print(ANDROID_LOG_INFO, "TOUCH", "BEGIN=[%.1f, %.1f]\n", x, y);
            break;
        case TOUCH_MOVE: {
            float vx = -(x - GTouchState.PosCurr.x);
            float vy = -(y - GTouchState.PosCurr.y);
            GTouchState.Velocity.x = GTouchState.cfg.Valpha*vx + (1-GTouchState.cfg.Valpha)*GTouchState.Velocity.x;
            GTouchState.Velocity.y = GTouchState.cfg.Valpha*vy + (1-GTouchState.cfg.Valpha)*GTouchState.Velocity.y;
            GTouchState.PosCurr.x = x;
            GTouchState.PosCurr.y = y;
            GTouchState.EventType = TOUCH_MOVE;
__android_log_print(ANDROID_LOG_INFO, "TOUCH", "MOVE=[%.1f, %.1f] v=%.1f\n", 
x, y, GTouchState.Velocity.y);
            break;
        }
        default:
            break;
    }
}
bool TouchScrollNextPos(const ImVec2& delta) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.HoveredWindow; //g.CurrentWindow;
    //ImRect rect = window->Rect();
/*__android_log_print(ANDROID_LOG_INFO, "imgui TOUCH", "Scroll=1, delta=[%.1f, %.1f]  rect=%.1f %.1f %.1f %.1f\n", 
 delta.x, delta.y,
 rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y);
//window->Rect().Min.x, window->Rect().Min.y, window->Rect().Max.x, window->Rect().Max.y);
*/
    if (window) { //g.HoveredId == 0) {
__android_log_print(ANDROID_LOG_INFO, "imgui TOUCH", "Scroll=1, delta=[%.1f, %.1f]  rect=%.1f %.1f %.1f %.1f\n", 
 delta.x, delta.y,
window->Rect().Min.x, window->Rect().Min.y, window->Rect().Max.x, window->Rect().Max.y);
        bool held = true;
        if (held && fabsf(delta.x) >= 1000.0f)
            ImGui::SetScrollX(window, window->Scroll.x + delta.x);
        if (held && fabsf(delta.y) >= 1.0f)
            ImGui::SetScrollY(window, window->Scroll.y + delta.y);
        if(GTouchState.ScrollLast.y == window->Scroll.y)
            return false;
        GTouchState.ScrollLast.x = window->Scroll.x;
        GTouchState.ScrollLast.y = window->Scroll.y;
    }
    return true;
}
void TouchScrollRun() {
    ImVec2 delta;
    switch(GTouchState.EventType) {
        case TOUCH_END:
            if(GTouchState.PosCurr.y != GTouchState.PosPrev.y
            || GTouchState.PosCurr.x != GTouchState.PosPrev.x) {
                float vx = -(GTouchState.PosCurr.x - GTouchState.PosPrev.x);
                float vy = -(GTouchState.PosCurr.y - GTouchState.PosPrev.y);
                GTouchState.Velocity.x += vx/2;//offset compensation
                GTouchState.Velocity.y += vy/2;//offset compensation
                GTouchState.PosPrev.x = GTouchState.PosCurr.x;
                GTouchState.PosPrev.y = GTouchState.PosCurr.y;
            }
            if(fabsf(GTouchState.Velocity.y) >= GTouchState.cfg.jitter) {
                float deceleration = -GTouchState.Velocity.y*GTouchState.cfg.Abeta;
                GTouchState.Velocity.y += deceleration;
                delta.x = 0;
                delta.y = GTouchState.Velocity.y;
                if(TouchScrollNextPos(delta) == false) //to end, done
                    GTouchState.Velocity.y = 0;
            }
            else
                GTouchState.Velocity.y = 0;
//__android_log_print(ANDROID_LOG_INFO, "TOUCH", "Run END v=%1.f\n", GTouchState.Velocity.y);
            break;
        case TOUCH_MOVE:
            delta.x = -(GTouchState.PosCurr.x - GTouchState.PosPrev.x);
            delta.y = -(GTouchState.PosCurr.y - GTouchState.PosPrev.y);
//__android_log_print(ANDROID_LOG_INFO, "TOUCH", "Run MOVE %.1f %.1f %.1f\n",
//delta.x, delta.y, GTouchState.cfg.step);
            GTouchState.PosPrev.x = GTouchState.PosCurr.x;
            GTouchState.PosPrev.y = GTouchState.PosCurr.y;
            if(fabsf(delta.y) >= GTouchState.cfg.jitter || fabsf(delta.x) >= GTouchState.cfg.jitter)
                TouchScrollNextPos(delta);
            break;
        default:
            break;
    }

}
#endif
};

