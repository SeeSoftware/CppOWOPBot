#include "LuaEnviornment.h"
#include <iostream>

#define NOMINMAX
#include <windows.h>

#include <vector>
#include <SFML/Graphics.hpp>

#include "SFExtraMath.h"
#include "Util.h"

bool LuaEnviornment::RunScript(const std::string & script, std::string *errorString)
{
	try
	{
		mState.safe_script(script);
		return true;
	}
	catch (const sol::error &)
	{
		return false;
	}
}

////Free functions
namespace LuaStuff
{
	template <typename T>
	std::string VecToString(const sf::Vector2<T> &self)
	{
		return "x: " + std::to_string(self.x) + " y: " + std::to_string(self.y);
	}

	std::string ColToString(const sf::Color &col)
	{
		return "r: " + std::to_string(col.r) + " g: " + std::to_string(col.g) + " b: " + std::to_string(col.b) + " a: " + std::to_string(col.a);
	}
}


inline void LuaPanic(sol::optional<std::string> maybe_msg)
{
	std::cerr << "Lua panic!" << std::endl;
	if (maybe_msg)
	{
		const std::string& msg = maybe_msg.value();
		std::cerr << "\terror message: " << msg << std::endl;

		MessageBox(NULL, msg.c_str(), "Lua panic!", NULL);
		return;
	}
	MessageBox(NULL, "No error message!", "Lua panic!", NULL);

	// When this function exits, Lua will exhibit default behavior and abort()
}


void LuaEnviornment::InitEnviornment()
{

	mState.set_exception_handler([](lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
	{
		std::cout << "An exception occurred in a function:\n";
		if (maybe_exception)
		{
			const std::exception& ex = *maybe_exception;
			std::cout << ex.what() << std::endl;
		}
		else
		{
			std::cout.write(description.data(), description.size());
			std::cout << std::endl;
		}

		return sol::stack::push(L, description);
	});

	mState.set_panic(sol::c_call<decltype(&LuaPanic), &LuaPanic>);

	mState.set_function("error_handler", [](std::string message) -> std::string
	{
		std::cout << "A function error occured: " << message << "\n";
		return message;
	});

	sol::protected_function::set_default_handler(mState["error_handler"]);

	mState.open_libraries(sol::lib::base);
	mState.open_libraries(sol::lib::math);
	mState.open_libraries(sol::lib::bit32);
	mState.open_libraries(sol::lib::coroutine);
	mState.open_libraries(sol::lib::string);
	mState.open_libraries(sol::lib::table);
	mState.open_libraries(sol::lib::utf8);
	mState.open_libraries(sol::lib::debug);

	mState.new_usertype<sf::Vector2i>("Vector2i",

		sol::constructors<sf::Vector2i(), sf::Vector2i(int, int), sf::Vector2i(const sf::Vector2f&)>(),

		"x", &sf::Vector2i::x,
		"y", &sf::Vector2i::y,

		"Distance", &sf::VectorDistance<int, int>,
		"Length", &sf::VectorLength<int>,
		"Dot", &sf::VectorDotProduct<int, int>,
		"Normalize", [](sf::Vector2i &self) { self = sf::VectorNormalize(self); },
		"GetNormalized", [](sf::Vector2i &self) -> sf::Vector2i { return sf::VectorNormalize(self); },
		"ToVec2f", [](sf::Vector2i &self) -> sf::Vector2f { return sf::Vector2f(self); },

		sol::meta_function::call, [](sf::Vector2i &self) { return std::make_tuple(self.x, self.y); }, //expands the vector to x,y
		sol::meta_function::to_string, &LuaStuff::VecToString<int>,
		sol::meta_function::addition, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a + b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a + sf::Vector2i(b); }),

		sol::meta_function::subtraction, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a - b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a - sf::Vector2i(b); }),

		sol::meta_function::division, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { if (b.x == 0 || b.y == 0) throw std::runtime_error("Division by 0"); return a / b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i{sf::Vector2i con = sf::Vector2i(b);  if (con.x == 0 || con.y == 0) throw std::runtime_error("Division by 0"); return a / con;},
			[](const sf::Vector2i &a, const int &b) -> sf::Vector2i { if (b == 0) throw std::runtime_error("Division by 0"); return a / b; }),

		sol::meta_function::multiplication, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a * b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a * sf::Vector2i(b); },
			[](const sf::Vector2i &a, const int &b) -> sf::Vector2i { return a * b; })

		);

	mState.new_usertype<sf::Vector2f>("Vector2f",

		sol::constructors<sf::Vector2f(), sf::Vector2f(float, float), sf::Vector2f(const sf::Vector2i&)>(),

		"x", &sf::Vector2f::x,
		"y", &sf::Vector2f::y,

		"Distance", &sf::VectorDistance<float, float>,
		"Length", &sf::VectorLength<float>,
		"Dot", &sf::VectorDotProduct<float, float>,
		"Normalize", [](sf::Vector2f &self) { self = sf::VectorNormalize(self); },
		"GetNormalized", [](sf::Vector2f &self) -> sf::Vector2f { return sf::VectorNormalize(self); },
		"ToVec2i", [](sf::Vector2f &self) -> sf::Vector2i { return sf::Vector2i(self); },

		sol::meta_function::call, [](sf::Vector2f &self) { return std::make_tuple(self.x, self.y); }, //expands the vector to x,y
		sol::meta_function::to_string, &LuaStuff::VecToString<float>,
		sol::meta_function::addition, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a + b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a + sf::Vector2f(b); }),

		sol::meta_function::subtraction, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a - b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a - sf::Vector2f(b); }),

		sol::meta_function::division, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a / b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a / sf::Vector2f(b); },
			[](const sf::Vector2f &a, const float &b) -> sf::Vector2f { return a / b; }),

		sol::meta_function::multiplication, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a * b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a * sf::Vector2f(b); },
			[](const sf::Vector2f &a, const float &b) -> sf::Vector2f { return a * b; })

		);


	mState.new_usertype<sf::Color>("CColor",

		sol::constructors<sf::Color(), sf::Color(uint8_t, uint8_t, uint8_t), sf::Color(uint8_t, uint8_t, uint8_t, uint8_t)>(),

		sol::meta_function::to_string, &LuaStuff::ColToString,
		"r", &sf::Color::r,
		"g", &sf::Color::g,
		"b", &sf::Color::b,
		"a", &sf::Color::a

		);

	mState.new_usertype<sf::Image>("CImage",
		sol::constructors <sf::Image()>(),

		"size", sol::property([](sf::Image &self) -> sf::Vector2i { return (sf::Vector2i)self.getSize(); }),
		"width", sol::property([](sf::Image &self) -> unsigned int { return self.getSize().x; }),
		"height", sol::property([](sf::Image &self) -> unsigned int { return self.getSize().y; }),

		"Create", [](sf::Image &self, unsigned int w, unsigned int h, const sf::Color &fillColor) { self.create(w, h, fillColor); },
		"LoadFromFile", [](sf::Image &self, std::string filename) { self.loadFromFile("Images/" + filename); }, //TODO: upwards traversing via /.. possible

		"SetPixel",[](sf::Image &self, unsigned int x, unsigned int y, const sf::Color &col) 
					{
						if (x >= self.getSize().x || y >= self.getSize().y)
							return;

						self.setPixel(x, y, col);
					},

		"GetPixel", [](sf::Image &self, unsigned int x, unsigned int y) -> sf::Color
					{
						if (x >= self.getSize().x || y >= self.getSize().y)
							return sf::Color(0,0,0,0);

						return self.getPixel(x, y);
					}
		
		);

	mState.new_usertype<sf::Texture>("CTexture",
		sol::constructors <sf::Texture()>(),

		"size", sol::property([](sf::Texture &self) -> sf::Vector2i { return (sf::Vector2i)self.getSize(); }),
		"width", sol::property([](sf::Texture &self) -> unsigned int { return self.getSize().x; }),
		"height", sol::property([](sf::Texture &self) -> unsigned int { return self.getSize().y; }),

		"LoadFromFile", [](sf::Texture &self, std::string filename) { self.loadFromFile("Images/" + filename); }, //TODO: upwards traversing via /.. possible
		"LoadFromImage", [](sf::Texture &self, const sf::Image &image) { self.loadFromImage(image); },

		"CopyToImage", [](sf::Texture &self) -> sf::Image { return self.copyToImage(); }

		);


	mState.set_function("Vec2i", [this](sol::variadic_args va) -> auto { return mState["Vector2i"]["new"](va); }); //proabaly unsafe but eh
	mState.set_function("Vec2f", [this](sol::variadic_args va) -> auto { return mState["Vector2f"]["new"](va); });
	mState.set_function("Color", [this](sol::variadic_args va) -> auto { return mState["CColor"]["new"](va); });
	mState.set_function("Image", [this](sol::variadic_args va) -> auto { return mState["CImage"]["new"](va); });
	mState.set_function("Texture", [this](sol::variadic_args va) -> auto { return mState["CTexture"]["new"](va); });

	mState.set_function("HSVColor", [](float h, float s, float v) -> sf::Color
	{
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
		return sf::Color((uint8_t)(r*255.0f), (uint8_t)(g*255.0f), (uint8_t)(b*255.0f));
	});

	//hook.Add / hook.Run / hook.Remove
	sol::table hooktbl = mState.create_named_table("hook");

	hooktbl.set_function("Add", 
		[this](std::string hookName, std::string functionName, const sol::protected_function &callback) {

			std::transform(hookName.begin(), hookName.end(), hookName.begin(), ::toupper);
			mHooks[hookName][functionName] = callback;

		});

	hooktbl.set_function("Remove",
		[this](std::string hookName, std::string functionName)
	{

		std::transform(hookName.begin(), hookName.end(), hookName.begin(), ::toupper);
		mHooks[hookName].erase(functionName);

	});

	hooktbl.set_function("Run",
		[this](std::string hookName, sol::variadic_args va)
	{
		std::transform(hookName.begin(), hookName.end(), hookName.begin(), ::toupper);
		RunHook(hookName, va);
	});

	///////////////////////////
	//render funcs
	static sf::Color mCurrentDrawCol = sf::Color(255,255,255);
	static sf::View mOldView;
	static sf::Texture *mCurrentTexture = nullptr;

	sol::table rendertbl = mState.create_named_table("render");
	rendertbl.set_function("SetDrawColor", [](const sf::Color &newCol){mCurrentDrawCol = newCol;});
	rendertbl.set_function("SetTexture", [](sf::Texture &texture) { mCurrentTexture = &texture; });
	
	rendertbl.set_function("StartGlobalView", [this]()
	{
		mOldView = mTarget.getView();
		mTarget.setView(mTarget.getDefaultView());
	});

	rendertbl.set_function("EndGlobalView", [this]()
	{
		mTarget.setView(mOldView);
	});



	rendertbl.set_function("DrawRect", [this](float x, float y, float w, float h)
	{
		sf::RectangleShape shape(sf::Vector2f(w, h));
		shape.setPosition(sf::Vector2f(x, y));
		shape.setFillColor(mCurrentDrawCol);
		mTarget.draw(shape);
	});

	rendertbl.set_function("DrawTexturedRect", [this](float x, float y, float w, float h)
	{
		sf::RectangleShape shape(sf::Vector2f(w, h));
		shape.setPosition(sf::Vector2f(x, y));
		shape.setFillColor(mCurrentDrawCol);
		shape.setTexture(mCurrentTexture);
		mTarget.draw(shape);
	});

	rendertbl.set_function("DrawRectOutline", [this](float x, float y, float w, float h, float thickness)
	{
		sf::RectangleShape shape(sf::Vector2f(w,h));
		shape.setPosition(sf::Vector2f(x,y));
		shape.setFillColor(sf::Color(0, 0, 0, 0));
		shape.setOutlineColor(mCurrentDrawCol);
		shape.setOutlineThickness(thickness);
		mTarget.draw(shape);
	});
	//////////

	//gui funcs
	sol::table imguitbl = mState.create_named_table("imgui");
	imguitbl.set_function("Begin", [](std::string title) -> bool
	{
		return ImGui::Begin(title == "" ? "None" : title.c_str());
	});

	imguitbl.set_function("End", []()
	{
		ImGui::End();
	});

	imguitbl.set_function("SameLine", [](std::string text, float spacing = -1.0f)
	{
		ImGui::SameLine(0.0f, spacing);
	});

	imguitbl.set_function("Text", [](std::string text)
	{
		ImGui::Text(text.c_str()); //TODO: string format exploit probably possible (the thing with %d %i %c %s etc)
	});

	imguitbl.set_function("InputText", [](std::string label, std::string text)->std::string
	{
		ImGui::InputText(label.c_str(),&text);
		return text;
	});

	imguitbl.set_function("InputDouble", [](std::string label, double num) -> double
	{
		ImGui::InputDouble(label.c_str(), &num);
		return num;
	});

	imguitbl.set_function("InputInt", [](std::string label, int num) -> int
	{
		ImGui::InputInt(label.c_str(), &num); 
		return num;
	});

	imguitbl.set_function("Combo", [](std::string label, int currentItem, std::vector<std::string> choices) -> int
	{
		currentItem -= 1;

		if (currentItem >= choices.size() || currentItem < 0)
			return 0;

		ImGui::Combo(label.c_str(), &currentItem, [](void* data, int index, const char **outText) -> bool 
		{
			std::vector<std::string> &container = *((std::vector<std::string>*)data);
			if (index >= container.size() || index < 0)
				return false;

			(*outText) = container[index].c_str();
			return true;
		
		}, (void*)&choices, (int)choices.size());

		currentItem += 1;
		return currentItem;
	});

	imguitbl.set_function("Button", [](std::string label) -> bool
	{
		return ImGui::Button(label.c_str());
	});

	imguitbl.set_function("ColorEdit3", [](std::string label, sf::Color color, bool noinputs = false) -> sf::Color
	{
		ImVec4 val = ImColor(color.r, color.g, color.b);
		ImGui::ColorEdit3(label.c_str(), (float*)&val, (noinputs ? ImGuiColorEditFlags_NoInputs : 0));
		return sf::Color((uint8_t)(val.x*255.0f), (uint8_t)(val.y*255.0f), (uint8_t)(val.z*255.0f));
	});

	imguitbl.set_function("ColorEdit4", [](std::string label, sf::Color color, bool noinputs = false) -> sf::Color
	{
		ImVec4 val = ImColor(color.r, color.g, color.b, color.a);
		ImGui::ColorEdit4(label.c_str(), (float*)&val, (noinputs ? ImGuiColorEditFlags_NoInputs : 0));
		return sf::Color((uint8_t)(val.x*255.0f), (uint8_t)(val.y*255.0f), (uint8_t)(val.z*255.0f), (uint8_t)(val.w*255.0f));
	});

	////////////
}
