#ifndef IMGUI_SFML_EXTRA_H
#define IMGUI_SFML_EXTRA_H

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-sfml.h"

namespace ImGui {

inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }


inline bool ImageButtonBlend(const sf::Texture& texture, const ImVec4& bg_col = ImColor(255, 255, 255, 255),
                             const ImVec4& drawCol_normal = ImColor(225, 225, 225, 255),
                             const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
                             const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = 0)
{
	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	const ImVec2& size = ImVec2(textureSize.x, textureSize.y);
	const ImVec2& uv0 = ImVec2(0, 0);
	const ImVec2& uv1 = ImVec2(1, 1);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	PushID((intptr_t)texture.getNativeHandle());
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);
	// Render
	/*const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));*/

	window->DrawList->AddImage((void *)(intptr_t)texture.getNativeHandle(), image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(
	                               (hovered && held) ? drawCol_Down : hovered ? drawCol_hover : drawCol_normal));

	return pressed;
}


inline bool ImageButtonAnim(const sf::Texture& texture, const sf::Texture& texture_hover, const sf::Texture& texture_down, 
	const ImVec4& bg_col = ImColor(255, 255, 255, 255),
                             int frame_padding = 0)
{
	const ImVec4 drawCol_normal = ImColor(225, 225, 225, 255);
	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	const ImVec2& size = ImVec2(textureSize.x, textureSize.y);
	const ImVec2& uv0 = ImVec2(0, 0);
	const ImVec2& uv1 = ImVec2(1, 1);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	PushID((intptr_t)texture.getNativeHandle());
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);
	// Render
	/*const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));*/
	void *tex = (hovered && held) ? (void *)(intptr_t)texture_down.getNativeHandle() : hovered ? (void *)(intptr_t)texture_hover.getNativeHandle() : (void *)(intptr_t)texture.getNativeHandle();

	window->DrawList->AddImage(tex, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(
	                               drawCol_normal));

	return pressed;
}

inline bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
                                const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImVec2 size = imageSize;
	if (size.x <= 0 && size.y <= 0) {size.x = size.y = ImGui::GetTextLineHeightWithSpacing();}
	else {
		if (size.x <= 0)          size.x = size.y;
		else if (size.y <= 0)     size.y = size.x;
		size = size * window->FontWindowScale * ImGui::GetIO().FontGlobalScale;
	}

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 textSize = ImGui::CalcTextSize(label, NULL, true);
	const bool hasText = textSize.x > 0;

	const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImVec2 totalSizeWithoutPadding(size.x + innerSpacing + textSize.x, size.y > textSize.y ? size.y : textSize.y);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding * 2);
	ImVec2 start(0, 0);
	start = window->DC.CursorPos + padding; if (size.y < textSize.y) start.y += (textSize.y - size.y) * .5f;
	const ImRect image_bb(start, start + size);
	start = window->DC.CursorPos + padding; start.x += size.x + innerSpacing; if (size.y > textSize.y) start.y += (size.y - textSize.y) * .5f;
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered = false, held = false;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

	window->DrawList->AddImage((void*)(intptr_t)texture.getNativeHandle(), image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

	if (textSize.x > 0) ImGui::RenderText(start, label);
	return pressed;
}


inline bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
                                int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
	sf::FloatRect textureRect = sf::FloatRect(0.f, 0.f, textureSize.x, textureSize.y);

	ImVec2 uv0(textureRect.left / textureSize.x, textureRect.top / textureSize.y);
	ImVec2 uv1((textureRect.left + textureRect.width)  / textureSize.x,
	           (textureRect.top  + textureRect.height) / textureSize.y);

	return ImageButtonWithText(texture, label, imageSize, uv0, uv1, frame_padding, bg_col, tint_col);
}

}
#endif