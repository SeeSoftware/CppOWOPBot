#include "LuaEnviornment.h"
#include <iostream>

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
	catch (const sol::error &err)
	{
		std::cout << err.what() << "\n";
		return false;
	}
}

////Free functions
namespace LuaStuff
{
	template <typename T>
	std::string VecToString(sf::Vector2<T> &self)
	{
		return "x: " + std::to_string(self.x) + " y: " + std::to_string(self.y);
	}
}

void LuaEnviornment::InitEnviornment()
{
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

		sol::meta_function::to_string, &LuaStuff::VecToString<int>,
		sol::meta_function::addition, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a + b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a + sf::Vector2i(b); }),

		sol::meta_function::subtraction, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a - b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a - sf::Vector2i(b); }),

		sol::meta_function::division, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { if (b.x == 0 || b.y == 0) throw sol::error("Division by 0"); return a / b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i{sf::Vector2i con = sf::Vector2i(b);  if (con.x == 0 || con.y == 0) throw sol::error("Division by 0"); return a / con;}),

		sol::meta_function::multiplication, sol::overload([](const sf::Vector2i &a, const sf::Vector2i &b) -> sf::Vector2i { return a * b; },
			[](const sf::Vector2i &a, const sf::Vector2f &b) -> sf::Vector2i { return a * sf::Vector2i(b); })

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

		sol::meta_function::to_string, &LuaStuff::VecToString<float>,
		sol::meta_function::addition, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a + b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a + sf::Vector2f(b); }),

		sol::meta_function::subtraction, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a - b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a - sf::Vector2f(b); }),

		sol::meta_function::division, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a / b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a / sf::Vector2f(b); }),

		sol::meta_function::multiplication, sol::overload([](const sf::Vector2f &a, const sf::Vector2f &b) -> sf::Vector2f { return a * b; },
			[](const sf::Vector2f &a, const sf::Vector2i &b) -> sf::Vector2f { return a * sf::Vector2f(b); })

		);


	mState.new_usertype<sf::Color>("CColor",

		sol::constructors<sf::Color(), sf::Color(uint8_t, uint8_t, uint8_t), sf::Color(uint8_t, uint8_t, uint8_t, uint8_t)>(),

		"r", &sf::Color::r,
		"g", &sf::Color::g,
		"b", &sf::Color::b,
		"a", &sf::Color::a

		);

	mState.set_function("Vec2i", [this](sol::variadic_args va) -> auto { return mState["Vector2i"]["new"](va); }); //proabaly unsafe but eh
	mState.set_function("Vec2f", [this](sol::variadic_args va) -> auto { return mState["Vector2f"]["new"](va); });
	mState.set_function("Color", [this](sol::variadic_args va) -> auto { return mState["CColor"]["new"](va); });

	//hook.Add / hook.Run / hook.Remove
	sol::table hooktbl = mState.create_named_table("hook");

	hooktbl.set_function("Add", 
		[this](std::string hookName, const std::string &functionName, const sol::function &callback) { 

			std::transform(hookName.begin(), hookName.end(), hookName.begin(), ::toupper);
			mHooks[hookName][functionName] = callback;

		});

	hooktbl.set_function("Remove",
		[this](std::string hookName, const std::string &functionName)
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
	static sf::Color mCurrentDrawCol;
	static sf::View mOldView;
	
	sol::table rendertbl = mState.create_named_table("render");
	rendertbl.set_function("SetDrawColor", [](const sf::Color &newCol){mCurrentDrawCol = newCol;});
	
	rendertbl.set_function("StartGlobalView", [this]()
	{
		mOldView = mTarget.getView();
		mTarget.setView(mTarget.getDefaultView());
	});

	rendertbl.set_function("EndGlobalView", [this]()
	{
		mTarget.setView(mOldView);
	});

	rendertbl.set_function("DrawRect", [this](const sf::Vector2f &pos, const sf::Vector2f &size)
	{
		sf::RectangleShape shape(size);
		shape.setPosition(pos);
		shape.setFillColor(mCurrentDrawCol);
		mTarget.draw(shape);
	});

	rendertbl.set_function("DrawRectOutline", [this](const sf::Vector2f &pos, const sf::Vector2f &size, float thickness)
	{
		sf::RectangleShape shape(size);
		shape.setPosition(pos);
		shape.setFillColor(sf::Color(0, 0, 0, 0));
		shape.setOutlineColor(mCurrentDrawCol);
		shape.setOutlineThickness(thickness);
		mTarget.draw(shape);
	});
	//////////

	//gui funcs
	sol::table guitbl = mState.create_named_table("gui");
	guitbl.set_function("Begin", [this](const std::string &title) -> bool
	{
		return ImGui::Begin(title == "" ? "None" : title.c_str());
	});

	guitbl.set_function("End", [this](const std::string &title)
	{
		ImGui::End();
	});

	guitbl.set_function("SameLine", [this](const std::string &text)
	{
		ImGui::SameLine();
	});

	guitbl.set_function("Text", [this](const std::string &text)
	{
		ImGui::Text(text.c_str()); //TODO: string format exploit probably possible (the thing with %d %i %c %s etc)
	});

	guitbl.set_function("InputText", [this](const std::string &label, std::string text)->std::string
	{
		ImGui::InputText(label.c_str(),&text);
		return text;
	});

	guitbl.set_function("InputDouble", [this](const std::string &label, double num) -> double
	{
		ImGui::InputDouble(label.c_str(), &num);
		return num;
	});

	guitbl.set_function("InputInt", [this](const std::string &label, int num) -> int
	{
		ImGui::InputInt(label.c_str(), &num); 
		return num;
	});

	///////
}
