
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "imgui-sfml-extra.h"

namespace ImGui {


bool ImageButtonBlend(const sf::Texture& texture, const ImVec4& bg_col,
                             const ImVec4& drawCol_normal,
                             const ImVec4& drawCol_hover,
                             const ImVec4& drawCol_Down, int frame_padding)
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


bool ImageButtonAnim(const sf::Texture& texture, const sf::Texture& texture_hover, const sf::Texture& texture_down,
                            const ImVec4& bg_col,
                            int frame_padding)
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

bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
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


bool ImageButtonWithText(const sf::Texture& texture, const char* label, const ImVec2& imageSize,
                                int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
	sf::FloatRect textureRect = sf::FloatRect(0.f, 0.f, textureSize.x, textureSize.y);

	ImVec2 uv0(textureRect.left / textureSize.x, textureRect.top / textureSize.y);
	ImVec2 uv1((textureRect.left + textureRect.width)  / textureSize.x,
	           (textureRect.top  + textureRect.height) / textureSize.y);

	return ImageButtonWithText(texture, label, imageSize, uv0, uv1, frame_padding, bg_col, tint_col);
}

void ImageWithText(const sf::Texture& texture, const char* fmt, ...) {
	ImVec2 current_pos = GetCursorScreenPos();
	Image(texture, static_cast<sf::Vector2f>(texture.getSize()), sf::Color::White, sf::Color::Transparent);
	ImVec2 image_pos = GetCursorScreenPos();

	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());

	va_list args;
	va_start(args, fmt);
	ImGuiContext& g = *GImGui;
	const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	ImVec2 textSize = CalcTextSize(g.TempBuffer, text_end);

	SetCursorScreenPos(ImVec2(current_pos.x + textureSize.x / 2 - textSize.x / 2, current_pos.y + textureSize.y / 2 - textSize.y / 2));

	TextUnformatted(g.TempBuffer, text_end);
	va_end(args);
		SetCursorScreenPos(image_pos);

};

void ImageNinePatch(const sf::Texture& texture, const sf::Vector2f& ssize) {
	ImVec2 size(ssize.x,ssize.y);
	ImVec4 tint_col = ImVec4(1,1,1,1);
	ImVec4 border_col = ImVec4(0,0,0,0);
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (border_col.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
	ImVec2 texSize(textureSize.x,textureSize.y);
	ImVec2 patchSize(textureSize.x/3.0f,textureSize.y/3.0f);

	ImVec2 topLeft(0.f, 0.f);
	ImVec2 top(patchSize.x, 0.f);
	ImVec2 topRight(patchSize.x*2.0f, 0.f);
	ImVec2 left(0.f, patchSize.y);
	ImVec2 center(patchSize.x, patchSize.y);
	ImVec2 centerRight(patchSize.x*2.0f, patchSize.y);
	ImVec2 bottomLeft(0.f, patchSize.y*2.0f);
	ImVec2 bottom(patchSize.x, patchSize.y*2.0f);
	ImVec2 bottomRight(patchSize.x*2.0f, patchSize.y*2.0f);

	float sizeW = (size.x - patchSize.x*2.0f);
	float sizeH = (size.y - patchSize.y*2.0f);

	ImVec2 topSize = ImVec2(sizeW, patchSize.y)/patchSize;
	ImVec2 centerSize = ImVec2(sizeW, sizeH)/patchSize;


	ImTextureID texId = (void*)texture.getNativeHandle();

	window->DrawList->AddImage(texId, bb.Min, bb.Min + patchSize, (topLeft/texSize), (topLeft + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + top, bb.Min + top + patchSize * topSize, (top/texSize), (top + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + top + ImVec2(sizeW,0.0f), bb.Min + top + ImVec2(sizeW,0.0f) + patchSize, (topRight/texSize), (topRight + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + left, bb.Min + left + top + ImVec2(0.0f, sizeH), (left/texSize), (left + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + center, bb.Min + center + patchSize * centerSize, (center/texSize), (center + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + center + ImVec2(sizeW, 0) , bb.Min + top + ImVec2(sizeW, sizeH) + patchSize, (centerRight/texSize), (centerRight + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + left + ImVec2(0, sizeH), bb.Min + left + ImVec2(0, sizeH) + patchSize, (bottomLeft/texSize), (bottomLeft + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + left + top + ImVec2(0, sizeH), bb.Min + left + ImVec2(sizeW, sizeH) + patchSize, (bottom/texSize), (bottom + patchSize)/texSize, GetColorU32(tint_col));
	window->DrawList->AddImage(texId, bb.Min + left + top + ImVec2(sizeW, sizeH), bb.Min + left + top + ImVec2(sizeW, sizeH) + patchSize, (bottomRight/texSize), (bottomRight + patchSize)/texSize, GetColorU32(tint_col));

}

void ImageNinePatchWithText(const sf::Texture& texture, const sf::Vector2f& ssize, const char* fmt, ...) {
	ImVec2 current_pos = GetCursorScreenPos();
	ImageNinePatch(texture, ssize);
	ImVec2 image_pos = GetCursorScreenPos();

	va_list args;
	va_start(args, fmt);
	ImGuiContext& g = *GImGui;
	const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	ImVec2 textSize = CalcTextSize(g.TempBuffer, text_end);

	SetCursorScreenPos(ImVec2(current_pos.x + ssize.x / 2 - textSize.x / 2, current_pos.y + ssize.y / 2 - textSize.y / 2));

	TextUnformatted(g.TempBuffer, text_end);
	va_end(args);
	SetCursorScreenPos(image_pos);
};

}