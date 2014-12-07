/*********************************************************************
Matt Marchant 2014
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

#include <BodyState.hpp>
#include <Util.hpp>

//------------------------------------------
void NpcStateAir::update(float dt)
{
    //reduce lateral velocity so sideways movement is minimal
    auto vel = getVelocity();
    vel.x *= 0.89f;
    setVelocity(vel);
}

void NpcStateAir::resolve(const sf::Vector3f& manifold, CollisionWorld::Body* other)
{
    switch (other->getType())
    {
    case CollisionWorld::Body::Type::Solid:
    case CollisionWorld::Body::Type::Block:
        //kill if block above and NPC is touching the ground
        if (manifold.y * manifold.z > 0 && getFootSenseCount() > 0)
        {
            kill();
            
            //raise event to say player killed us
            game::Event e;
            e.node.action = game::Event::NodeEvent::KilledNode;
            e.node.type = Category::Block;
            e.node.target = Category::Npc;

            if ((e.node.type & Category::LastTouchedOne) || (e.node.type & Category::GrabbedOne)) e.node.owner = Category::PlayerOne;
            else if ((e.node.type & Category::LastTouchedTwo) || (e.node.type & Category::GrabbedTwo)) e.node.owner = Category::PlayerTwo;
            else e.node.owner = Category::None;

            e.type = game::Event::Node;
            raiseEvent(e, other); //this should reference the other body as the sender not the NPC
        }

        move(sf::Vector2f( manifold.x, manifold.y ) * manifold.z);
    {   
        if (getFootSenseCount() > 0 && manifold.y * manifold.z < 0)
            setState<NpcStateGround>();
        //jump away from walls, or switch to ground sitting
        auto vel = getVelocity();       
        //if (manifold.x != 0)
        vel.x = -vel.x;
        vel.y = 0.f;
        setVelocity(vel);
    }
        break;
    case CollisionWorld::Body::Type::Player:
    {
        //kill self if player is above
        if (manifold.y * manifold.z > 0)
        {
            move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
            auto vel = getVelocity();
            vel.y = 0.f;
            setVelocity(vel);
            if (getFootSenseCount() > 0)
            {
                kill();
                
                //raise event to say player killed us
                game::Event e;
                e.node.action = game::Event::NodeEvent::KilledNode;
                e.node.type = other->getParentCategory();
                e.node.target = Category::Npc;
                e.type = game::Event::Node;
                raiseEvent(e, other); //this should reference the other body as the sender not the NPC
            }
        }
    }
    break;
    case CollisionWorld::Body::Type::Npc:
        //steer away from other baddies
        move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
        {
            auto vel = getVelocity();
            if (manifold.y != 0)
            {
                vel.y = -vel.y;
                vel.x = Util::Random::value(200.f, 400.f);
                if (Util::Random::value(0, 1) == 0)
                {
                    vel.x = -vel.x;
                }
            }
            if (manifold.x != 0)
            {
                vel.x = -vel.x;
            }
            setVelocity(vel);
        }
        break;

    default: break;
    }
}

//-------------------------------------------

NpcStateGround::NpcStateGround(CollisionWorld::Body* b)
    : BodyState         (b),
    m_jumpDelay         (Util::Random::value(1.f, 2.f)),
    m_accumulatedTime   (0.f)
{
    setVelocity({});
}

void NpcStateGround::update(float dt)
{
    auto fsc = getFootSenseCount();
    
    auto vel = getVelocity();
    if(fsc > 0) vel.y = 0.f;
    vel.x *= getFriction();
       
    m_accumulatedTime += dt;
    if (m_accumulatedTime > m_jumpDelay
        && fsc > 0)
    {
        vel.y = -900.f;
        setState<NpcStateAir>();
    }

    setVelocity(vel);
}

void NpcStateGround::resolve(const sf::Vector3f& manifold, CollisionWorld::Body* other)
{
    switch (other->getType())
    {
    case CollisionWorld::Body::Type::Block:
        if (/*Util::Vector::lengthSquared(getVelocity()) > 0.2f
            && */(manifold.x != 0.f || (manifold.y * manifold.z < 0))) //prevents shifting vertically down
        {
            move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
            setVelocity({});
            m_accumulatedTime = 0.f;
        }
        else if (manifold.y * manifold.z > 0)
        {
            //block is above, so crush
            kill();

            //raise event to say player killed us
            game::Event e;
            e.node.action = game::Event::NodeEvent::KilledNode;
            e.node.type = Category::Block;
            e.node.target = Category::Npc;

            if ((e.node.type & Category::LastTouchedOne) || (e.node.type & Category::GrabbedOne)) e.node.owner = Category::PlayerOne;
            else if ((e.node.type & Category::LastTouchedTwo) || (e.node.type & Category::GrabbedTwo)) e.node.owner = Category::PlayerTwo;
            else e.node.owner = Category::None;

            e.type = game::Event::Node;
            raiseEvent(e, other);
        }
        {
            int cat = other->getParentCategory();
            if (cat & (Category::GrabbedOne | Category::GrabbedTwo | Category::LastTouchedOne | Category::LastTouchedTwo))
                damage(std::fabs(manifold.z * 0.3f), other);
        }
        break;
    case CollisionWorld::Body::Type::Solid:
        move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
        setVelocity({});
        break;
    case CollisionWorld::Body::Type::Player:
        //kill self if player is above
        if (manifold.y * manifold.z > 0)
        {
            kill();

            //raise event to say player killed us
            game::Event e;
            e.node.action = game::Event::NodeEvent::KilledNode;
            e.node.type = other->getParentCategory();
            e.node.target = Category::Npc;
            e.type = game::Event::Node;
            raiseEvent(e, other); //this should reference the other body as the sender not the NPC
        }
    case CollisionWorld::Body::Type::Npc:
        if (Util::Vector::lengthSquared(getVelocity()) > 0.2f
            && manifold.x != 0.f) //prevents shifting vertically
        {
            move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
            setVelocity({});
        }
        break;
    default: break;
    }
}