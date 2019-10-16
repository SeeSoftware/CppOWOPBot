#include "ImageTool.h"
#include "Imgui/IncImgui.h"
#include "UInterface/UInterface.h"

void ImageTool::DrawGui()
{
	if (ImGui::Begin("Image Tool"))
	{
		ImGui::InputText("Filename", &mFilename);
		if (ImGui::Button("Load"))
		{
			mImage.loadFromFile(mFilename);
			mPreview.loadFromImage(mImage);
		}
	}
	ImGui::End();


	//Preview
	sf::Vector2f cursorPos = mTarget.mapPixelToCoords(sf::Mouse::getPosition(mTarget));

	sf::RectangleShape shape((sf::Vector2f)mImage.getSize());
	shape.setTexture(&mPreview);
	shape.setPosition(cursorPos);
	shape.setFillColor(sf::Color(255, 255, 255, 128));

	mTarget.draw(shape);
}

void ImageTool::ProcessEvent(const sf::Event & e)
{
	if (mUi.IsViewActive() && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Button::Left)
	{
		sf::Vector2f cursorPos = mTarget.mapPixelToCoords(sf::Vector2i(e.mouseButton.x, e.mouseButton.y));

		//Monsterous statement
		for (int i = 512; i >= 1; i /= 2)
			for (uint32_t x = 0; x < mImage.getSize().x; x++)
				for (uint32_t y = 0; y < mImage.getSize().y; y++)
					for (int d = 0; d < 2; d++)
						if (((d == 0) ? (x + y) : (x - y)) % i == 0)
							mManager.GetTaskManager().PushTask(Task(Task::PlacePixel, (sf::Vector2i)cursorPos + sf::Vector2i(x, y), mImage.getPixel(x, y)));

		SortPixelTasks();
	}
}

void ImageTool::Update(float dt)
{
}

std::string ImageTool::ToolName() const
{
	return "Image Tool";
}

sf::Texture &ImageTool::ToolImage() const
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

void ImageTool::SortPixelTasks()
{
	mManager.GetTaskManager().Update([](UnsafeTaskManager &tman)
	{
		auto &container = tman.GetTasks()[Task::PlacePixel];

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

			return a < b;
		});
	});


}
