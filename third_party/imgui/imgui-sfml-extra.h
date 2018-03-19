#ifndef IMGUI_SFML_EXTRA_H
#define IMGUI_SFML_EXTRA_H

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-sfml.h"

namespace ImGui {

//inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
//inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
//inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }


bool ImageButtonBlend(const sf::Texture& texture, const ImVec4& bg_col = ImColor(255, 255, 255, 255),
                             const ImVec4& drawCol_normal = ImColor(225, 225, 225, 255),
                             const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
                             const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = 0);

bool ImageButtonAnim(const sf::Texture& texture, const sf::Texture& texture_hover, const sf::Texture& texture_down,
                            const ImVec4& bg_col = ImColor(255, 255, 255, 255),
                            int frame_padding = 0);

bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
                                const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
                                int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

void ImageWithText(const sf::Texture& texture, const char* fmt, ...);

void ImageNinePatch(const sf::Texture& texture, const sf::Vector2f& size);
void ImageNinePatchWithText(const sf::Texture& texture, const sf::Vector2f& ssize, const char* fmt, ...);
}

#endif