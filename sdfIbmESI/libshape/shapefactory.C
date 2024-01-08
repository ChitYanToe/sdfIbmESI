#include "ishape.H"
#include "shapefactory.H"

#include <iostream>

namespace sdfibm
{
    #define REGISTERSHAPE(m) \
    bool sdfibm::m::added = ShapeFactory::add(sdfibm::m::typeName(), sdfibm::m::create);

    bool ShapeFactory::add(const string& name, TCreateMethod create_method)
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

    IShape* ShapeFactory::create(const string& name, const dictionary& para)
{
    auto it = m_methods.find(name);
    if(it != m_methods.end())
    {
        return it->second(para);
    }
    return nullptr;
}

	std::map<string, ShapeFactory::TCreateMethod> ShapeFactory::m_methods;

// add shapes
#include "circle.H"
REGISTERSHAPE(Circle);
#include "sphere.H"
REGISTERSHAPE(Sphere);
#include "ellipse.H"
REGISTERSHAPE(Ellipse);
#include "ellipsoid.H"
REGISTERSHAPE(Ellipsoid);
#include "rectangle.H"
REGISTERSHAPE(Rectangle);
#include "circle_tail.H"
REGISTERSHAPE(Circle_Tail);
#include "circle_twotail.H"
REGISTERSHAPE(Circle_TwoTail);
#include "box.H"
REGISTERSHAPE(Box);
#include "plane.H"
REGISTERSHAPE(Plane);
}
