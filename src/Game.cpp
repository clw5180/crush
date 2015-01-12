/*********************************************************************
Matt Marchant 2014 - 2015
http://trederia.blogspot.com

Crush - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <Game.hpp>
#include <TitleState.hpp>
#include <MenuState.hpp>
#include <GameState.hpp>
#include <PauseState.hpp>
#include <GameOverState.hpp>
#include <ConsoleState.hpp>
#include <Util.hpp>
#include <Resource.hpp>
#include <FileSystem.hpp>

#include <SFML/Graphics/Font.hpp>

namespace
{
    const float timePerFrame = 1.f / 60.f;

    sf::Clock frameClock;
    float timeSinceLastUpdate = 0.f;

    GameData gameData;
}

Game::Game()
    : m_renderWindow    (sf::VideoMode(1024, 576), "Crush", sf::Style::Close), //1024, 576
    m_stateStack        (State::Context(m_renderWindow, *this, gameData)),
    m_paused            (false),
    m_console           (getFont("res/fonts/VeraMono.ttf"))
{
    registerStates();
    m_stateStack.pushState(States::ID::Title);

    m_renderWindow.setVerticalSyncEnabled(true);

    m_clearColour = sf::Color(100u, 149u, 237u);

    //bind commands to console
    registerConCommands();
}

//public
void Game::run()
{
    //load default console config
    m_console.exec("bind tab show");
    m_console.exec("bind escape quit");
    m_console.exec("exec default.con");

    frameClock.restart();
    while (m_renderWindow.isOpen())
    {
        float elapsedTime = frameClock.restart().asSeconds();
        timeSinceLastUpdate += elapsedTime;

        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;

            handleEvents();
            if(!m_paused)
                update(timePerFrame);
        }
        draw();
    }

    //write console config file
    m_console.exec("export_config");
}

void Game::pause()
{
    m_paused = true;
}

void Game::resume()
{
    m_paused = false;
    frameClock.restart();
    timeSinceLastUpdate = 0.f;
}

void Game::setClearColour(sf::Color c)
{
    m_clearColour = c;
}

sf::Font& Game::getFont(const std::string& path)
{
    return m_fontResource.get(path);
}

TextureResource& Game::getTextureResource()
{
    return m_textureResource;
}

ShaderResource& Game::getShaderResource()
{
    return m_shaderResource;
}

Console& Game::getConsole()
{
    return m_console;
}

void Game::playMusic(const std::string& title, bool loop)
{
    m_musicPlayer.play(title, loop);
}

void Game::stopMusic()
{
    m_musicPlayer.stop();
}

void Game::pauseMusic()
{
    m_musicPlayer.setPaused(true);
}

void Game::resumeMusic()
{
    m_musicPlayer.setPaused(false);
}

//private
void Game::handleEvents()
{
    sf::Event evt;
    while (m_renderWindow.pollEvent(evt))
    {
        //pause simulation while window is not focused
        if (evt.type == sf::Event::LostFocus)
            pause();
        else if (evt.type == sf::Event::GainedFocus)
            resume();

        m_console.handleEvent(evt);
        m_stateStack.handleEvent(evt);
        
        if(evt.type == sf::Event::Closed)
            m_renderWindow.close();
    }
}

void Game::update(float dt)
{
    m_stateStack.update(dt);
}

void Game::draw()
{
    m_renderWindow.clear(m_clearColour);
    m_stateStack.draw();
    if (m_console.visible()) m_renderWindow.draw(m_console);
    m_renderWindow.display();
}

void Game::registerStates()
{
    m_stateStack.registerState<TitleState>(States::ID::Title);
    m_stateStack.registerState<MenuState>(States::ID::Menu);
    m_stateStack.registerState<GameState>(States::ID::Game);
    m_stateStack.registerState<PauseState>(States::ID::Pause);
    m_stateStack.registerState<GameOverState>(States::ID::GameOver);
    m_stateStack.registerState<ConsoleState>(States::ID::Console);
}

void Game::registerConCommands()
{
    //-------toggle console state-----//
    Console::CommandData cd;
    cd.action = [this](Console::CommandList l)->std::string
    {
        if (m_console.visible())
        {
            m_stateStack.popState();
            return "";
        }
        else
        {
            m_stateStack.pushState(States::ID::Console);
            return "";
        }
    };
    cd.help = "toggles console visibility";
    m_console.addItem("show", cd);

    //-----quit application-----//
    cd.action = [this](Console::CommandList l)->std::string
    {
        m_renderWindow.close();
        return "Quitting...";
    };
    cd.help = "quits the game";
    m_console.addItem("quit", cd);

    //----register map change----//
    cd.action = [this](Console::CommandList l)->std::string
    {
        if (l.empty()) return "please specify a map name";
        for (auto i = 0u; i < gameData.mapList.size(); ++i)
        {
            if (Util::String::toLower(gameData.mapList[i]) == Util::String::toLower(l[0]) + ".crm")
            {
                gameData.mapIndex = i;
                m_stateStack.clearStates();
                m_stateStack.pushState(States::ID::Game);
                return "changing map to " + l[0];
            }
        }
        return "map " + l[0] + " not found.";
    };
    cd.help = "load specified map";
    m_console.addItem("map", cd);

    //---skip map---//
    cd.action = [this](Console::CommandList l)->std::string
    {
        gameData.mapIndex++;
        if (gameData.mapIndex == gameData.mapList.size())
            gameData.mapIndex = 0;

        m_stateStack.clearStates();
        m_stateStack.pushState(States::ID::Game);
        return "";
    };
    cd.help = "load next map in the list";
    m_console.addItem("nextmap", cd);
}