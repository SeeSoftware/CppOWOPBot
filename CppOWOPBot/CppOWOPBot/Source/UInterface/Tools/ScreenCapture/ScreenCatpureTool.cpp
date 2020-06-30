#include "ScreenCatpureTool.h"
#include "Imgui/IncImgui.h"
#include "ScreenCaptureHelper.h"
#define NOMINMAX
#include <Windows.h>

static const std::array<sf::Color, 16> CONSOLE_COLORS{
	sf::Color(0,0,0),
	sf::Color(0,0,170),
	sf::Color(0,170,0),
	sf::Color(0,170,170),
	sf::Color(170,0,0),
	sf::Color(170,0,170),
	sf::Color(170,85,0),
	sf::Color(170,170,170),
	sf::Color(85,85,85),
	sf::Color(85,85,255),
	sf::Color(85,255,85),
	sf::Color(85,255,255),
	sf::Color(255,85,85),
	sf::Color(255,85,255),
	sf::Color(255,255,85),
	sf::Color(255,255,255),
};

ScreenCaptureTool::ScreenCaptureTool(UInterface& ui, BotManager& manager, sf::RenderWindow& target) : ITool(ui, manager, target), mIsEnabled(false) { mHWND = GetDesktopWindow(); mImageSize = ScreenCaptureHelper::getWindowSize(mHWND); }

void ScreenCaptureTool::DrawGui()
{
	if (ImGui::Begin("Screen Capture"))
	{
		ImGui::Checkbox("Enabled", &mIsEnabled);
		ImGui::Checkbox("Reduce Colors", &mReduceColors);
		if(mReduceColors)
			ImGui::Checkbox("Dither colors", &mDitherColors);

		ImGui::InputInt("HWND", (int*)&mHWND);

		if (ImGui::Button("Reset HWND"))
			mHWND = GetDesktopWindow();


		ImGui::InputInt("x", &mImagePos.x);
		ImGui::InputInt("y", &mImagePos.y);

		ImGui::InputInt("w", &mImageSize.x);
		ImGui::InputInt("h", &mImageSize.y);

		if (ImGui::Button("Reset Size"))
		{
			mImageSize = ScreenCaptureHelper::getWindowSize(mHWND);
			if (mImageSize.x > 2000)
				mImageSize.x = 2000;
			if (mImageSize.y > 2000)
				mImageSize.y = 2000;
		}
	}
	ImGui::End();

	if (ImGui::Begin("Preview"))
	{
		static sf::Texture previewImage;
		if (mPreviewClock.getElapsedTime().asSeconds() > 0.5)
		{
			previewImage.loadFromImage(CaptureImage());
			mPreviewClock.restart();
		}

		ImGui::Image(previewImage);
	}
	ImGui::End();

	if (ImGui::Begin("HWND List"))
	{
		static std::vector< std::tuple <std::string, HWND>> result;

		static const auto enumWindowCallback = [](HWND hWnd, LPARAM lparam) -> BOOL
		{
			auto vec = (std::vector< std::tuple <std::string, HWND>>*)lparam;

			int length = GetWindowTextLength(hWnd);
			char* buffer = new char[length + 1];
			GetWindowText(hWnd, buffer, length + 1);
			std::string windowTitle(buffer);

			// List visible windows with a non-empty title
			if (IsWindowVisible(hWnd) && length != 0)
				vec->push_back(std::make_tuple(windowTitle, hWnd));

			return TRUE;
		};

		static sf::Clock hwndUpdate;
		if (hwndUpdate.getElapsedTime().asSeconds() > 10)
		{
			result.clear();
			EnumWindows(enumWindowCallback, (LPARAM)&result);
			hwndUpdate.restart();
		}

		for (auto& [str, hwnd] : result)
			if (ImGui::Button(str.c_str()))
				mHWND = hwnd;

	}	
	ImGui::End();

}

void ScreenCaptureTool::ProcessEvent(const sf::Event& e)
{
}

void ScreenCaptureTool::Update(float dt)
{
	if (!mIsEnabled)
		return;

	if (mManager.GetTaskManager().GetNumTasks(Task::PlacePixel) > 0)
		return;

	sf::Image image = CaptureImage();

	//Draw image in owop;
	//FIX: I DONT FUCKING UNDERSTAND WHY THE IMAGE IS Y MIRRORED SO HERE IS A HAXKY FIX I DONT CARE ANYMORE
	mManager.GetWorld().Update([&](UnsafeWorld& world)
	{
		mManager.GetTaskManager().Update([&](UnsafeTaskManager& manager)
			{
				for (uint32_t y = 0; y < image.getSize().y; y++)
					for (uint32_t x = 0; x < image.getSize().x; x++)
					{
						sf::Color imagePixel = image.getPixel(x, y);
						if (world.GetPixel(mImagePos + sf::Vector2i(x, y)) == imagePixel)
							continue;

						manager.PushTask(Task(Task::PlacePixel, mImagePos + sf::Vector2i(x, y), imagePixel));
					}
			});
	});

	SortPixelTasks();
}

std::string ScreenCaptureTool::ToolName() const
{
	return "Screen Capture";
}

sf::Texture& ScreenCaptureTool::ToolImage() const
{
	static sf::Texture texture;
	static bool loaded = false;
	if (!loaded)
	{
		texture.loadFromFile("Images/ImageTool.png");
		loaded = true;
	}

	return texture;
}

/*void reduceDepth(sf::Color& col, uint32_t colorsPerComponent)
{
	uint32_t divisor = 256 / colorsPerComponent;
	uint32_t rounding = divisor / 2;

	col.r = (uint8_t)(((uint32_t)col.r + rounding) / divisor) * divisor;
	col.g = (uint8_t)(((uint32_t)col.g + rounding) / divisor) * divisor;
	col.b = (uint8_t)(((uint32_t)col.b + rounding) / divisor) * divisor;
}*/


float getColorDistance(const sf::Color& a, const sf::Color b)
{
	return sqrtf(((float)a.r - (float)b.r) * ((float)a.r - (float)b.r) + ((float)a.g - (float)b.g) * ((float)a.g - (float)b.g) + ((float)a.b - (float)b.b) * ((float)a.b - (float)b.b));
}

template<size_t S>
sf::Color getNearestColor(const sf::Color& col, const std::array<sf::Color, S>& pallete)
{
	sf::Color nearest;
	float dist = INFINITY;
	for (const sf::Color& x : pallete)
	{
		float newDist = getColorDistance(col,x);
		if (newDist <= dist)
		{
			dist = newDist;
			nearest = x;
		}
	}

	return nearest;
}

bool checkBounds(const sf::Image& img, uint32_t x, uint32_t y)
{
	if (x >= img.getSize().x || y >= img.getSize().y)
		return false;

	return true;
}

size_t xyToIndex(uint32_t x, uint32_t y, uint32_t w)
{
	return (x + (size_t)y * (size_t)w) * 4;
}

sf::Image ScreenCaptureTool::CaptureImage()
{
	/*static std::array<sf::Color, 64> COLOR_MAP;
	static bool first = true;

	if (first)
	{
		for (size_t i = 0; i < COLOR_MAP.size(); i++)
		{
			COLOR_MAP[i] = sf::Color((i & 0x3) * 63, (i & (0x3 << 2)) * 63, (i & (0x3 << 4)) * 63);
		}
	}*/



	sf::Image orgImg = ScreenCaptureHelper::hwndToImage(mHWND, mImageSize);
	
	if (mReduceColors)
	{

		float* imageError = new float[orgImg.getSize().x * orgImg.getSize().y * 4];
		memset(imageError, 0, orgImg.getSize().x * orgImg.getSize().y * 4 * sizeof(float));
		uint32_t w = orgImg.getSize().x;

		static const std::array<sf::Vector2i, 4> POS_THING{
			sf::Vector2i(1,0),sf::Vector2i(-1,1),sf::Vector2i(0,1),sf::Vector2i(1,1)
		};
		static const std::array<float, 4> ERROR_MUL{
			7.0f / 16.0f,3.0f / 16.0f,5.0f / 16.0f,1.0f / 16.0f
		};

		for (uint32_t y = 0; y < orgImg.getSize().y; y++)
			for (uint32_t x = 0; x < orgImg.getSize().x; x++)
			{
				sf::Color orgCol = orgImg.getPixel(x, y);
				sf::Color errorCol = orgCol;
				
				if (mDitherColors)
				{
					errorCol.r = (uint8_t)std::max(std::min(((float)errorCol.r + imageError[xyToIndex(x, y, w)]), 255.0f), 0.0f);
					errorCol.g = (uint8_t)std::max(std::min(((float)errorCol.g + imageError[xyToIndex(x, y, w) + 1]), 255.0f), 0.0f);
					errorCol.b = (uint8_t)std::max(std::min(((float)errorCol.b + imageError[xyToIndex(x, y, w) + 2]), 255.0f), 0.0f);
				}

				sf::Color col = getNearestColor(errorCol, CONSOLE_COLORS);
				orgImg.setPixel(x, y, col);

				float rError = (float)orgCol.r - (float)col.r;
				float gError = (float)orgCol.g - (float)col.g;
				float bError = (float)orgCol.b - (float)col.b;

				for (size_t i = 0; i < POS_THING.size(); i++)
				{
					sf::Vector2i pos = POS_THING[i];
					float mul = ERROR_MUL[i];

					if (checkBounds(orgImg, x + pos.x, y + pos.y))
					{
						imageError[xyToIndex(x + pos.x, y + pos.y, w)] += rError * mul;
						imageError[xyToIndex(x + pos.x, y + pos.y, w) + 1] += gError * mul;
						imageError[xyToIndex(x + pos.x, y + pos.y, w) + 2] += bError * mul;
					}
				}

			}

		delete[] imageError;
	}

	return orgImg;
}

void ScreenCaptureTool::SortPixelTasks()
{
	mManager.GetTaskManager().Update([](UnsafeTaskManager& tman)
		{
			auto& container = tman.GetTasks()[Task::PlacePixel];

			/*std::sort(container.begin(), container.end(), [&, this](const Task& lhs, const Task& rhs) -> bool
			{
				sf::Vector2i lpos = mBotController->GetWorld().WorldPosToChunkPos(lhs.pos) / sf::Vector2i(3, 3);
				sf::Vector2i rpos = mBotController->GetWorld().WorldPosToChunkPos(rhs.pos) / sf::Vector2i(3, 3);
				std::pair<int, int> a(lpos.x, lpos.y);
				std::pair<int, int> b(rpos.x, rpos.y);

				return a < b;
			});*/

			std::sort(container.begin(), container.end(), [&](const Task& lhs, const Task& rhs) -> bool
				{
					sf::Color lcol = lhs.color;
					sf::Color rcol = rhs.color;
					std::tuple<uint8_t, uint8_t, uint8_t> a(lcol.r, lcol.g, lcol.b);
					std::tuple<uint8_t, uint8_t, uint8_t> b(rcol.r, rcol.g, rcol.b);

					return a > b;
				});
		});
}
