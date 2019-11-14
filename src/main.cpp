#include "ofMain.h"
#include "random_graph.hpp"

int main()
{
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new RandomGraph());
}
