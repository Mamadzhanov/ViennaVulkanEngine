
#include <iostream>
#include <functional>

#include "jobSystemTest.h"
#include "STLTest.h"
#include "VEInclude.h"

using namespace vve;

int main()
{
    std::cout << "Hello World!\n";

    JADD( jst::jobSystemTest() );

    //vec::testVector();
    //tab::testTables();
    //stltest::runSTLTests();

    vgjs::JobSystem::getInstance()->wait();
    vgjs::JobSystem::getInstance()->terminate();
    vgjs::JobSystem::getInstance()->waitForTermination();


    return 0;

    syseng::init();
    syseng::computeOneFrame();
    syseng::close();
 
    return 0;
}

