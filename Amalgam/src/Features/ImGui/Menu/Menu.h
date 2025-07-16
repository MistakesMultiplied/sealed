#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>
#include <vector>
#include <random>

struct Output_t
{
	std::string m_sFunction;
	std::string m_sLog;
	size_t m_iID;

	Color_t tAccent;
};

struct MatrixDrop_t
{
	float x;
	float y;
	float speed;
	float length;
	float alpha;
	char character;
	
	MatrixDrop_t() : x(0), y(0), speed(0), length(0), alpha(0), character(0) {}
	MatrixDrop_t(float _x, float _y, float _speed, float _length, float _alpha, char _char) 
		: x(_x), y(_y), speed(_speed), length(_length), alpha(_alpha), character(_char) {}
};

class MatrixBackground
{
private:
	std::vector<MatrixDrop_t> m_vDrops;
	std::mt19937 m_rng;
	std::uniform_real_distribution<float> m_distX;
	std::uniform_real_distribution<float> m_distSpeed;
	std::uniform_real_distribution<float> m_distLength;
	std::uniform_real_distribution<float> m_distAlpha;
	std::uniform_int_distribution<int> m_distChar;
	
	float m_flLastTime;
	int m_iMaxDrops;
	ImVec2 m_vScreenSize;
	
	const char* m_szCharacters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

public:
	MatrixBackground() : m_rng(std::random_device{}()), m_flLastTime(0), m_iMaxDrops(120)
	{
		ResetDistributions();
	}
	
	void ResetDistributions()
	{
		m_distX = std::uniform_real_distribution<float>(0, m_vScreenSize.x);
		m_distSpeed = std::uniform_real_distribution<float>(50, 150);
		m_distLength = std::uniform_real_distribution<float>(12, 35);
		m_distAlpha = std::uniform_real_distribution<float>(0.4f, 1.0f);
		m_distChar = std::uniform_int_distribution<int>(0, 61);
	}
	
	void Update(float flDeltaTime, const ImVec2& vScreenSize)
	{
		m_vScreenSize = vScreenSize;
		ResetDistributions();
		
		for (auto it = m_vDrops.begin(); it != m_vDrops.end();)
		{
			it->y += it->speed * flDeltaTime;
			it->alpha -= flDeltaTime * 0.3f;
			
			if (it->y > m_vScreenSize.y + it->length || it->alpha <= 0 || it->y < -it->length)
				it = m_vDrops.erase(it);
			else
				++it;
		}
		
		if (m_vDrops.size() < m_iMaxDrops)
		{
			if (m_rng() % 100 < 15)
			{
				float x = m_distX(m_rng);
				float speed = m_distSpeed(m_rng);
				float length = m_distLength(m_rng);
				float alpha = m_distAlpha(m_rng);
				char character = m_szCharacters[m_distChar(m_rng)];
				
				bool bTooClose = false;
				for (const auto& existingDrop : m_vDrops)
				{
					if (abs(existingDrop.x - x) < 20.0f && existingDrop.y > -50.0f)
					{
						bTooClose = true;
						break;
					}
				}
				
				if (!bTooClose)
				{
					m_vDrops.emplace_back(x, -length, speed, length, alpha, character);
				}
			}
		}
	}
	
	void Render(ImDrawList* pDrawList)
	{
		// Draw dark background overlay
		ImColor bgColor(0.0f, 0.0f, 0.0f, 0.7f);
		pDrawList->AddRectFilled(
			ImVec2(0, 0),
			m_vScreenSize,
			bgColor
		);
		
		for (const auto& drop : m_vDrops)
		{
			// Main character with bright green and glow effect
			ImColor glowColor(0.0f, 0.8f, 0.0f, drop.alpha * 0.5f);
			ImColor color(0.0f, 1.0f, 0.0f, drop.alpha);
			
			// Draw glow effect (multiple layers for better glow)
			for (int glow = 0; glow < 3; glow++)
			{
				float glowAlpha = drop.alpha * 0.2f / (glow + 1);
				ImColor multiGlowColor(0.0f, 0.8f, 0.0f, glowAlpha);
				
				pDrawList->AddText(
					ImVec2(drop.x + glow, drop.y + glow),
					multiGlowColor,
					&drop.character
				);
			}
			
			// Draw main character
			pDrawList->AddText(
				ImVec2(drop.x, drop.y),
				color,
				&drop.character
			);
			
			// Trailing characters with fading effect
			for (int i = 1; i < drop.length; i++)
			{
				float trailAlpha = drop.alpha * (1.0f - (float)i / drop.length) * 0.8f;
				ImColor trailColor(0.0f, 0.8f, 0.0f, trailAlpha);
				ImColor trailGlowColor(0.0f, 0.6f, 0.0f, trailAlpha * 0.4f);
				
				// Use consistent character for trail based on position
				char trailChar = m_szCharacters[(m_distChar(m_rng) + i) % 62];
				
				// Draw trail glow
				pDrawList->AddText(
					ImVec2(drop.x + 1, drop.y - i * 18 + 1),
					trailGlowColor,
					&trailChar
				);
				
				// Draw trail character
				pDrawList->AddText(
					ImVec2(drop.x, drop.y - i * 18),
					trailColor,
					&trailChar
				);
			}
		}
	}
	
	void Clear()
	{
		m_vDrops.clear();
		m_flLastTime = 0;
	}
};

class CMenu
{
private:
	MatrixBackground m_MatrixBG;
	float m_flLastFrameTime;
	
	void DrawMenu();
	void DrawMatrixBackground();

	void MenuAimbot(int iTab);
	void MenuVisuals(int iTab);
	void MenuMisc(int iTab);
	void MenuLogs(int iTab);
	void MenuSettings(int iTab);
	void MenuSearch(std::string sSearch);

	void AddDraggable(const char* sLabel, ConfigVar<DragBox_t>& var, bool bShouldDraw = true, ImVec2 vSize = { H::Draw.Scale(100), H::Draw.Scale(40) });
	void AddResizableDraggable(const char* sLabel, ConfigVar<WindowBox_t>& var, bool bShouldDraw = true, ImVec2 vMinSize = { H::Draw.Scale(100), H::Draw.Scale(100) }, ImVec2 vMaxSize = { H::Draw.Scale(1000), H::Draw.Scale(1000) }, ImGuiSizeCallback fCustomCallback = nullptr);
	void DrawBinds();

	std::deque<Output_t> m_vOutput = {};
	size_t m_iMaxOutputSize = 1000;

public:
	void Render();
	void AddOutput(const std::string& sFunction, const std::string& sLog, const Color_t& tColor = Vars::Menu::Theme::Accent.Value);

	bool m_bIsOpen = false;
	bool m_bInKeybind = false;
	bool m_bWindowHovered = false;
};

ADD_FEATURE(CMenu, Menu);