#include "imotion.H"
#include "motionfactory.H"
#include <iostream>

namespace sdfibm
{
#define REGISTERMOTION(m) \
    bool sdfibm::m::added = MotionFactory::add(sdfibm::m::typeName(), sdfibm::m::create);

    bool MotionFactory::add(const string& name, TCreateMethod create_method)
{
    auto it = m_methods.find(name);
    // std::cout << "Registering..." << name << std::endl;
    if(it == m_methods.end())
    {
        m_methods[name] = create_method;
        return true;
    }
    return false;
}

    IMotion* MotionFactory::create(const string& name, const dictionary& node)
{
    auto it = m_methods.find(name);
    if(it != m_methods.end())
    {
        return it->second(node);
    }
    return nullptr;
}

std::map<string, MotionFactory::TCreateMethod> MotionFactory::m_methods;

}
// add motions
#include "motion000002.H"
REGISTERMOTION(Motion000002);
#include "motion110002.H"
REGISTERMOTION(Motion110002);
#include "motion222000.H"
REGISTERMOTION(Motion222000);
#include "motionsinedirectional.H"
REGISTERMOTION(MotionSineDirectional);
#include "motion01mask.H"
REGISTERMOTION(Motion01Mask);
#include "motionrotor.H"
REGISTERMOTION(MotionRotor);
#include "motionopenclose.H"
REGISTERMOTION(MotionOpenClose);
