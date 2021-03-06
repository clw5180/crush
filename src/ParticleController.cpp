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

#include <ParticleController.hpp>
#include <ParticleShaders.hpp>
#include <Node.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <iostream>

namespace
{
    //actually looks better to pick one of these at random
    //rather than trying to get random floats in such a large range
    std::vector<sf::Vector2f> splatVelocities =
    {
        { -200.f, -500.f },
        { 0.f, -400.f },
        { -250.f, -600.f },
        { -100.f, -1000.f },
        { 70.f, -900.f },
        { 200.f, -500.f },
        { -300.f, -400.f },
        { 50.f, -450.f },
        { -200.f, -350.f },
        { -174.f, -1050.f },
        { 380.f, -900.f },
    };

    std::vector<sf::Vector2f> splashVelocities = 
    {
        {-25.f, -600.f},
        {-14.f, -700.f},
        {-8.f, -640.f},
        {0.f, -1080.f},
        {7.f, -720.f},
        {18.f, -670.f},
        {29.f, -680.f}
    };

    std::vector<sf::Vector2f> puffVelocities = 
    {
        {-80.5f, 0.f},
        {-60.f, -8.9f},
        {-20.f, -24.f},
        {0.f, -40.5f},
        {28.5f, -24.6f},
        {64.f, -8.5f},
        {80.9f, 0.f},
        {64.f, 9.5f},
        {28.f, 27.5f},
        {0.7f, 40.4f},
        {-20.f, 29.6f},
        {-60.f, 9.5f}
    };

    std::vector<sf::Vector2f> smokeVelocities = 
    {
        {-30.f, -20.f},
        {-20.f, -30.f},
        {0.f, -40.f},
        {20.f, -30.f},
        {30.f, -20.f},
        {-50.f, -60.f},
        {-40.f, -70.f},
        {-30.f, -40.f},
        {30.f, -45.f},
        {40.f, -60.f}
    };

    std::vector<sf::Vector2f> sparkVelocities =
    {
        { -180.5f, 0.f },
        { -120.f, -88.9f },
        { -40.f, -124.f },
        { 0.f, -120.5f },
        { 48.5f, -64.6f },
        { 124.f, -88.5f },
        { 160.9f, 0.f },
        { 124.f, 9.5f },
        { 48.f, 27.5f },
        { 0.7f, 40.4f },
        { -40.f, 29.6f },
        { -120.f, 9.5f }
    };
}

ParticleController::ParticleController(TextureResource& tr, ShaderResource& sr)
    : m_textureResource (tr),
    m_shaderResource    (sr)
{
    m_systems.reserve(50);
}

//public
void ParticleController::update(float dt)
{
    for (auto& p : m_systems)
        p.update(dt);
}

void ParticleController::onNotify(Subject& s, const Event& evt)
{
    if (evt.type == Event::Node)
    {
        switch (evt.node.action)
        {
        case Event::NodeEvent::Despawn:
        {
            switch (evt.node.type)
            {
            case Category::Npc:
            {
                auto& ps = findSystem(Particle::Type::Splat);
                ps.setPosition({ evt.node.positionX, evt.node.positionY });
                ps.start(6u, 0.1f);
            }
                break;
            case Category::PlayerOne:
            {
                auto& ps = findSystem(Particle::Type::PlayerOneDie);
                ps.setPosition({ evt.node.positionX, evt.node.positionY });
                ps.start(1u, 0.1f);
            }
                break;
            case Category::PlayerTwo:
            {
                auto& ps = findSystem(Particle::Type::PlayerTwoDie);
                ps.setPosition({ evt.node.positionX, evt.node.positionY });
                ps.start(1u, 0.1f);
            }
                break;
            default:
            {
                auto& ps = findSystem(Particle::Type::Puff);
                ps.setPosition({ evt.node.positionX, evt.node.positionY });
                ps.start(5u, 0.02f);
            }
                break;
            }
            break;
        }
        break;
        case Event::NodeEvent::HitWater:
        {
            //splish!
            auto& ps = findSystem(Particle::Type::Splash);
            ps.setPosition({ evt.node.positionX, evt.node.positionY });
            ps.start(4u, 0.02f);
        }
        break;
        case Event::NodeEvent::Spawn:
        {
            if (evt.node.type != Category::Npc)
            {
                //do dust puff
                auto& ps = findSystem(Particle::Type::Puff);
                ps.setPosition({ evt.node.positionX, evt.node.positionY });
                ps.start(5u, 0.1f);
            }
        }
        break;
        case Event::NodeEvent::WentTurbo:
            //std::cout << "went turbo" << std::endl;
        {
            /*auto& ps = findSystem(Particle::Type::Smoke);
            ps.setNode(static_cast<Node&>(s));
            ps.start();*/
        }
            break;
        /*case Event::NodeEvent::LeftTurbo:
            std::cout << "left turbo" << std::endl;
            break;*/
        default: break;
        }
    }
    else if (evt.type == Event::Player)
    {
        switch (evt.player.action)
        {
        case Event::PlayerEvent::GotHat:
        {
            auto& ps = findSystem(Particle::Type::Sparkle);
            ps.setNode(static_cast<Node&>(s));
            ps.start();
        }
            break;
        default: break;
        }
    }
}

//private
ParticleSystem& ParticleController::addSystem(Particle::Type type)
{
    m_systems.emplace_back(type);
    ParticleSystem& particleSystem = m_systems.back();
    switch (type)
    {
    case Particle::Type::Splat:
        {
            particleSystem.setTexture(m_textureResource.get("res/textures/particles/gear.png"));
            particleSystem.setNormalMap(m_textureResource.get("res/textures/particles/gear_normal.png"));
            particleSystem.setRandomInitialVelocity(splatVelocities);
            particleSystem.setShader(m_shaderResource.get(Shader::Type::Metal));

            ForceAffector fa({ 0.f, 3500.f }); //gravity
            particleSystem.addAffector(fa);
            RotateAffector ra(380.f);
            particleSystem.addAffector(ra);
            ScaleAffector sa({ 5.5f, 5.5f });
            particleSystem.addAffector(sa);
        }
        break;
    case  Particle::Type::Splash:
        {
            particleSystem.setTexture(m_textureResource.get("res/textures/particles/water_splash.png"));
            particleSystem.setNormalMap( m_textureResource.get("res/textures/particles/water_splash_normal.png"));
            particleSystem.setShader(m_shaderResource.get(Shader::Type::WaterDrop));
            particleSystem.setColour({ 96u, 172u, 222u, 190u });
            particleSystem.setParticleLifetime(1.2f);
            particleSystem.setParticleSize({ 4.f, 9.f });
            particleSystem.setRandomInitialVelocity(splashVelocities);

            ForceAffector fa({ 0.f, 1500.f }); //gravity
            particleSystem.addAffector(fa);

            ScaleAffector sa({ 1.f, 8.5f });
            particleSystem.addAffector(sa);
            
            particleSystem.setBlendMode(sf::BlendAlpha);
        }
        break;
    case Particle::Type::Puff:
        particleSystem.setTexture(m_textureResource.get("res/textures/particles/dust_puff.png"));
        particleSystem.setShader(m_shaderResource.get(Shader::Type::FlatShaded));
        particleSystem.setParticleLifetime(1.f);
        particleSystem.setParticleSize({ 10.f, 10.f });
        particleSystem.setRandomInitialVelocity(puffVelocities);
        {
            ForceAffector fa({ 0.f, -20.f });
            particleSystem.addAffector(fa);

            ScaleAffector sa({ 4.f, 2.f });
            particleSystem.addAffector(sa);

            RotateAffector ra(40.f);
            particleSystem.addAffector(ra);
        }
        break;
    case Particle::Type::PlayerOneDie: //TODO p1 and p2 are rather similar....
    {
        auto texture = m_textureResource.get("res/textures/particles/player_one_particle.png");
        particleSystem.setTexture(m_textureResource.get("res/textures/particles/player_one_particle.png"));
        particleSystem.setShader(m_shaderResource.get(Shader::Type::FlatShaded));
        particleSystem.setParticleLifetime(2.f);
        particleSystem.setParticleSize(sf::Vector2f(texture.getSize()));
        particleSystem.setInitialVelocity({ 12.f, -100.f });

        ForceAffector fa({ 0.f, 20.f });
        particleSystem.addAffector(fa);
    }
        break;
    case Particle::Type::PlayerTwoDie:        
    {
        auto texture = m_textureResource.get("res/textures/particles/player_two_particle.png");
        particleSystem.setTexture(m_textureResource.get("res/textures/particles/player_two_particle.png"));
        particleSystem.setShader(m_shaderResource.get(Shader::Type::FlatShaded));
        particleSystem.setParticleLifetime(2.f);
        particleSystem.setParticleSize(sf::Vector2f(texture.getSize()));
        particleSystem.setInitialVelocity({ 12.f, -100.f });

        ForceAffector fa({ 0.f, 20.f });
        particleSystem.addAffector(fa);

    }
    break;
    case Particle::Type::Smoke:
    {
        particleSystem.setTexture(m_textureResource.get("res/textures/particles/dust_puff.png"));
        particleSystem.setShader(m_shaderResource.get(Shader::Type::FlatShaded));
        particleSystem.setParticleLifetime(3.f);
        particleSystem.setParticleSize({ 10.f, 10.f });
        particleSystem.setRandomInitialVelocity(smokeVelocities);
        particleSystem.setEmitRate(20.f);
        
        ForceAffector fa({ 10.f, -60.f });
        particleSystem.addAffector(fa);

        ScaleAffector sa({ 10.f, 10.f });
        particleSystem.addAffector(sa);

        RotateAffector ra(40.f);
        particleSystem.addAffector(ra);
        
    }
        break;
    case Particle::Type::Sparkle:
        particleSystem.setTexture(m_textureResource.get("res/textures/particles/sparkle.png"));
        particleSystem.setShader(m_shaderResource.get(Shader::Type::FlatShaded));
        particleSystem.setParticleLifetime(0.5f);
        particleSystem.setParticleSize({ 10.f, 10.f });
        particleSystem.setRandomInitialVelocity(sparkVelocities);
        {
            ForceAffector fa({ 0.f, 20.f });
            particleSystem.addAffector(fa);

            ScaleAffector sa({ 2.f, 2.f });
            particleSystem.addAffector(sa);

            RotateAffector ra(140.f);
            particleSystem.addAffector(ra);
        }
        break;
    default: break;
    }
    return particleSystem;
}

ParticleSystem& ParticleController::findSystem(Particle::Type type)
{
    //find an idle system and use it
    auto result = std::find_if(m_systems.begin(), m_systems.end(),
        [type](const ParticleSystem& ps)
    {
        return (ps.getType() == type && !ps.started());
    });

    if (result != m_systems.end())
    {
        return (*result); 
    }

    //else append a new system
    else
    {
        return addSystem(type);
    }
}

void ParticleController::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //only draw visible.
    for (auto& p : m_systems) 
        if(p.getParticleCount() > 0) 
            rt.draw(p, states);
}