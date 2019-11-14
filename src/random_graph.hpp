#pragma once

#include "ofMain.h"
#include <random>

template <class T>
struct Hash
{
	size_t operator()(T x) const noexcept
	{
		using type = typename underlying_type<T>::type;
		return hash<type>{}(static_cast<type>(x));
	}
};

class RandomGraph : public ofBaseApp
{
	enum class GraphType
	{
		ErdosRenyi,
		BarabasiAlbert,
		WattsStrogatz
	};

public:
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int) override;

	ofVec3f generateNode(float, float);
	void generateErdosRenyi(int, float, float, float);
	void generateBarabasiAlbert(int, float, float, int);
	void generateWattsStrogatz(int, float, float, int, float);

	std::vector<ofVec3f> mNodes;
	std::vector<std::tuple<int, int>> mEdges;
	std::vector<ofVec2f> mVertices;

	GraphType mGraphType;
	std::unordered_map<std::string, float> mParams;

	ofTrueTypeFont mLargeFont;
	ofTrueTypeFont mSmallFont;
	ofEasyCam mCamera;
	ofShader mShader;

	float mEdgeProb;
	int mNumEdges;
	int mNumNeighbors;
	float mRewireProb;

	std::random_device mSeed;
	std::mt19937 mEngine;
};

void RandomGraph::setup()
{
	mParams = {{"largeFontSize", 20},
			   {"smallFontSize", 10},
			   {"edgeWidth", 0.1},
			   {"nodeRadius", 0.1},
			   {"numNodes", 100},
			   {"radiusMean", 100.0},
			   {"radiusStd", 10.0},
			   {"edgeProbMin", 0.05},
			   {"edgeProbMax", 0.2},
			   {"numEdgesMin", 5},
			   {"numEdgesMax", 20},
			   {"numNeighborsMin", 5},
			   {"numNeighborsMax", 20},
			   {"rewireProbMin", 0.005},
			   {"rewireProbMax", 0.02},
			   {"cameraPositionX", 1000.0},
			   {"cameraPositionY", 1000.0},
			   {"cameraPositionZ", 1000.0},
			   {"cameraTargetX", 0.0},
			   {"cameraTargetY", 0.0},
			   {"cameraTargetZ", 0.0}};

	mGraphType = GraphType::WattsStrogatz;
	mNumNeighbors = std::uniform_int_distribution<>(mParams["numNeighborsMin"], mParams["numNeighborsMax"])(mEngine);
	mRewireProb = std::uniform_real_distribution<>(mParams["rewireProbsMin"], mParams["rewireProbMax"])(mEngine);
	generateWattsStrogatz(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumNeighbors, mRewireProb);

	ofBackground(0);
	ofEnableDepthTest();
	ofEnableSmoothing();
	ofSetLineWidth(mParams["edgeWidth"]);
	mLargeFont.load("Helvetica", mParams["largeFontSize"]);
	mSmallFont.load("Helvetica", mParams["smallFontSize"]);
	mShader.load("", "shader.flag");
	mCamera.setAutoDistance(false);
	mCamera.setPosition(ofPoint(mParams["cameraPositionX"], mParams["cameraPositionY"], mParams["cameraPositionZ"]));
	mCamera.setTarget(ofPoint(mParams["cameraTargetX"], mParams["cameraTargetY"], mParams["cameraTargetZ"]));
}

void RandomGraph::update()
{
	mVertices.clear();
	for (const auto &world : mNodes)
	{
		auto screen = mCamera.worldToScreen(world);
		mVertices.emplace_back(screen.x, ofMap(screen.y, 0, ofGetHeight(), ofGetHeight(), 0));
	}
}

void RandomGraph::draw()
{
	ofSetColor(255);
	switch (mGraphType)
	{
	case GraphType::ErdosRenyi:
		mLargeFont.drawString("Erdos Renyi", 100, 100);
		mSmallFont.drawString("Edge Prob: " + std::to_string(mEdgeProb), ofGetWidth() - 200, 100);
		break;
	case GraphType::BarabasiAlbert:
		mLargeFont.drawString("Barabasi Albert", 100, 100);
		mSmallFont.drawString("Num Edges: " + std::to_string(mNumEdges), ofGetWidth() - 200, 100);
		break;
	case GraphType::WattsStrogatz:
		mLargeFont.drawString("Watts Strogatz", 100, 100);
		mSmallFont.drawString("Num Neighbors: " + std::to_string(mNumNeighbors), ofGetWidth() - 200, 100);
		mSmallFont.drawString("Rewire Prob: " + std::to_string(mRewireProb), ofGetWidth() - 200, 120);
		break;
	}
	mSmallFont.drawString("e: Erdos Renyi", ofGetWidth() - 200, ofGetHeight() - 140);
	mSmallFont.drawString("b: Barabasi Albert", ofGetWidth() - 200, ofGetHeight() - 120);
	mSmallFont.drawString("w: Watts Strogatz", ofGetWidth() - 200, ofGetHeight() - 100);
	mCamera.begin();
	ofPushMatrix();
	ofRotate(ofGetElapsedTimef() * 10);
	for (const auto &node : mNodes)
	{
		ofDrawSphere(node, mParams["nodeRadius"]);
	}
	for (const auto &edge : mEdges)
	{
		ofDrawLine(mNodes[std::get<0>(edge)], mNodes[std::get<1>(edge)]);
	}
	ofPopMatrix();
	mCamera.end();
	mShader.begin();
	mShader.setUniform2fv("vertices", &mVertices[0][0], mVertices.size());
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	mShader.end();
}

ofVec3f RandomGraph::generateNode(float radiusMean, float radiusStd)
{
	auto radius = std::normal_distribution<>(radiusMean, radiusStd)(mEngine);
	auto theta = std::uniform_real_distribution<>(-M_PI, +M_PI)(mEngine);
	auto phi = std::uniform_real_distribution<>(-M_PI, +M_PI)(mEngine);

	return ofVec3f(radius * std::sin(theta) * std::cos(phi),
				   radius * std::sin(theta) * std::sin(phi),
				   radius * std::cos(theta));
}

void RandomGraph::generateErdosRenyi(int numNodes, float radiusMean, float radiusStd, float edgeProb)
{
	mNodes.clear();
	for (auto i = 0; i < numNodes; ++i)
	{
		mNodes.emplace_back(generateNode(radiusMean, radiusStd));
	}

	mEdges.clear();
	for (auto i = 0; i < numNodes; ++i)
	{
		for (auto j = 0; j < i; ++j)
		{
			if (std::bernoulli_distribution(edgeProb)(mEngine))
			{
				mEdges.emplace_back(i, j);
			}
		}
	}
}

void RandomGraph::generateBarabasiAlbert(int numNodes, float radiusMean, float radiusStd, int numEdges)
{
	generateErdosRenyi(numEdges, radiusMean, radiusStd, 1.0);

	for (auto i = numEdges; i < numNodes; ++i)
	{
		std::vector<int> degrees(mNodes.size(), 0);
		for (const auto &edge : mEdges)
		{
			++degrees[std::get<0>(edge)];
			++degrees[std::get<1>(edge)];
		}

		for (auto j = 0; j < numEdges; ++j)
		{
			mEdges.emplace_back(i, std::discrete_distribution<>(degrees.begin(), degrees.end())(mEngine));
		}

		mNodes.emplace_back(generateNode(radiusMean, radiusStd));
	}
}

void RandomGraph::generateWattsStrogatz(int numNodes, float radiusMean, float radiusStd, int numNeighbors, float rewireProb)
{
	mNodes.clear();
	for (auto i = 0; i < numNodes; ++i)
	{
		mNodes.emplace_back(generateNode(radiusMean, radiusStd));
	}

	mEdges.clear();
	for (auto i = 0; i < numNodes; ++i)
	{
		std::vector<std::tuple<int, float>> norms;
		for (auto j = 0; j < numNodes; ++j)
		{
			norms.emplace_back(j, mNodes[j].distance(mNodes[i]));
		}
		std::sort(norms.begin(), norms.end(), [](const std::tuple<int, float> &norm1, const std::tuple<int, float> &norm2) {
			return std::get<1>(norm1) < std::get<1>(norm2);
		});
		for (auto j = 0; j < numNeighbors; ++j)
		{
			if (std::bernoulli_distribution(rewireProb)(mEngine))
			{
				mEdges.emplace_back(i, std::uniform_int_distribution<>(0, numNodes - 1)(mEngine));
			}
			else
			{
				mEdges.emplace_back(i, std::get<0>(norms[j]));
			}
		}
	}
}

void RandomGraph::keyPressed(int key)
{
	switch (key)
	{
	case 'e':
	{
		mGraphType = GraphType::ErdosRenyi;
		mEdgeProb = std::uniform_real_distribution<>(mParams["edgeProbMin"], mParams["edgeProbMax"])(mEngine);
		generateErdosRenyi(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mEdgeProb);
	}
	break;
	case 'b':
	{
		mGraphType = GraphType::BarabasiAlbert;
		mNumEdges = std::uniform_int_distribution<>(mParams["numEdgesMin"], mParams["numEdgesMax"])(mEngine);
		generateBarabasiAlbert(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumEdges);
	}
	break;
	case 'w':
	{
		mGraphType = GraphType::WattsStrogatz;
		mNumNeighbors = std::uniform_int_distribution<>(mParams["numNeighborsMin"], mParams["numNeighborsMax"])(mEngine);
		mRewireProb = std::uniform_real_distribution<>(mParams["rewireProbsMin"], mParams["rewireProbMax"])(mEngine);
		generateWattsStrogatz(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumNeighbors, mRewireProb);
	}
	break;
	}
}