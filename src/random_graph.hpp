#pragma once

#include "ofMain.h"
#include <random>

class RandomGraph : public ofBaseApp
{
	enum class GraphType
	{
		ErdosRenyi,
		BarabasiAlbert,
		WattsStrogatz
	};

	struct Node
	{
		ofVec3f mPosition;
		ofVec3f mVelocity;
		ofVec3f mAcceleration;
	};

	struct Edge
	{
		int mHead;
		int mTail;
		float mLength;
		float mWeight;
	};

public:
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int) override;

	Node generateNode(float, float);
	void generateErdosRenyi(int, float, float, float);
	void generateBarabasiAlbert(int, float, float, int);
	void generateWattsStrogatz(int, float, float, int, float);

	std::vector<Node> mNodes;
	std::vector<Edge> mEdges;
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
			   {"edgeWidth", 0.01},
			   {"nodeRadius", 0.1},
			   {"numNodes", 100},
			   {"radiusMean", 100.0},
			   {"radiusStd", 10.0},
			   {"edgeProbMin", 0.05},
			   {"edgeProbMax", 0.2},
			   {"numEdgesMin", 1},
			   {"numEdgesMax", 10},
			   {"numNeighborsMin", 10},
			   {"numNeighborsMax", 20},
			   {"rewireProbMin", 0.01},
			   {"rewireProbMax", 0.1},
			   {"edgeWeightMin", 0.0},
			   {"edgeWeightMax", 0.1},
			   {"perlinNoiseNorm", 10.0},
			   {"deltaTime", 0.1},
			   {"cameraPositionX", 1000.0},
			   {"cameraPositionY", 1000.0},
			   {"cameraPositionZ", 1000.0},
			   {"cameraTargetX", 0.0},
			   {"cameraTargetY", 0.0},
			   {"cameraTargetZ", 0.0}};

	mGraphType = GraphType::WattsStrogatz;
	mNumNeighbors = std::uniform_int_distribution<int>(mParams["numNeighborsMin"], mParams["numNeighborsMax"])(mEngine);
	mRewireProb = std::uniform_real_distribution<float>(mParams["rewireProbsMin"], mParams["rewireProbMax"])(mEngine);
	generateWattsStrogatz(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumNeighbors, mRewireProb);

	ofBackground(240);
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
	for (auto &node : mNodes)
	{
		node.mAcceleration = ofVec3f(ofSignedNoise(node.mPosition.x, node.mPosition.y, node.mPosition.z),
									 ofSignedNoise(node.mPosition.y, node.mPosition.z, node.mPosition.x),
									 ofSignedNoise(node.mPosition.z, node.mPosition.x, node.mPosition.y)) *
							 mParams["perlinNoiseNorm"];
	}

	for (const auto &edge : mEdges)
	{
		auto direction = mNodes[edge.mTail].mPosition - mNodes[edge.mHead].mPosition;
		auto stretch = direction - direction.getNormalized() * edge.mLength;
		mNodes[edge.mHead].mAcceleration += edge.mWeight * stretch;
		mNodes[edge.mTail].mAcceleration -= edge.mWeight * stretch;
	}

	for (auto &node : mNodes)
	{
		node.mVelocity += node.mAcceleration * mParams["deltaTime"];
		node.mPosition += node.mVelocity * mParams["deltaTime"] + 0.5 * node.mAcceleration * mParams["deltaTime"] * mParams["deltaTime"];
	}

	mVertices.clear();
	for (const auto &node : mNodes)
	{
		auto position = mCamera.worldToScreen(node.mPosition);
		mVertices.emplace_back(position.x, ofMap(position.y, 0, ofGetHeight(), ofGetHeight(), 0));
	}
}

void RandomGraph::draw()
{
	ofSetColor(0);
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
	ofSetColor(0);
	for (const auto &node : mNodes)
	{
		ofDrawSphere(node.mPosition, mParams["nodeRadius"]);
	}
	for (const auto &edge : mEdges)
	{
		ofSetColor(0, 0, 0, 255 * (1 - edge.mWeight / mParams["edgeWeightMax"]));
		ofDrawLine(mNodes[edge.mHead].mPosition, mNodes[edge.mTail].mPosition);
	}
	mCamera.end();

	mShader.begin();
	mShader.setUniform2fv("vertices", &mVertices[0][0], mVertices.size());
	ofSetColor(0);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	mShader.end();
}

RandomGraph::Node RandomGraph::generateNode(float radiusMean, float radiusStd)
{
	auto radius = std::normal_distribution<float>(radiusMean, radiusStd)(mEngine);
	auto theta = std::uniform_real_distribution<float>(-M_PI, +M_PI)(mEngine);
	auto phi = std::uniform_real_distribution<float>(-M_PI, +M_PI)(mEngine);

	return Node{ofVec3f(radius * std::sin(theta) * std::cos(phi),
						radius * std::sin(theta) * std::sin(phi),
						radius * std::cos(theta))};
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
				auto weight = std::uniform_real_distribution<float>(mParams["edgeWeightMin"], mParams["edgeWeightMax"])(mEngine);
				mEdges.emplace_back(Edge{i, j, mNodes[i].mPosition.distance(mNodes[j].mPosition), weight});
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
			++degrees[edge.mHead];
			++degrees[edge.mTail];
		}

		mNodes.emplace_back(generateNode(radiusMean, radiusStd));

		for (auto j = 0; j < numEdges; ++j)
		{
			auto k = std::discrete_distribution<int>(degrees.begin(), degrees.end())(mEngine);
			auto weight = std::uniform_real_distribution<float>(mParams["edgeWeightMin"], mParams["edgeWeightMax"])(mEngine);
			mEdges.emplace_back(Edge{i, k, mNodes[i].mPosition.distance(mNodes[k].mPosition), weight});
		}
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
		std::vector<std::pair<float, int>> norms;
		for (auto j = 0; j < numNodes; ++j)
		{
			norms.emplace_back(mNodes[j].mPosition.distance(mNodes[i].mPosition), j);
		}
		std::sort(norms.begin(), norms.end());
		for (auto j = 0; j < numNeighbors; ++j)
		{
			auto weight = std::uniform_real_distribution<float>(mParams["edgeWeightMin"], mParams["edgeWeightMax"])(mEngine);
			if (std::bernoulli_distribution(rewireProb)(mEngine))
			{
				auto k = std::uniform_int_distribution<int>(0, numNodes - 1)(mEngine);
				mEdges.emplace_back(Edge{i, k, mNodes[i].mPosition.distance(mNodes[k].mPosition), weight});
			}
			else
			{
				mEdges.emplace_back(Edge{i, norms[j].second, mNodes[i].mPosition.distance(mNodes[norms[j].second].mPosition), weight});
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
		mEdgeProb = std::uniform_real_distribution<float>(mParams["edgeProbMin"], mParams["edgeProbMax"])(mEngine);
		generateErdosRenyi(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mEdgeProb);
	}
	break;
	case 'b':
	{
		mGraphType = GraphType::BarabasiAlbert;
		mNumEdges = std::uniform_int_distribution<int>(mParams["numEdgesMin"], mParams["numEdgesMax"])(mEngine);
		generateBarabasiAlbert(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumEdges);
	}
	break;
	case 'w':
	{
		mGraphType = GraphType::WattsStrogatz;
		mNumNeighbors = std::uniform_int_distribution<int>(mParams["numNeighborsMin"], mParams["numNeighborsMax"])(mEngine);
		mRewireProb = std::uniform_real_distribution<float>(mParams["rewireProbsMin"], mParams["rewireProbMax"])(mEngine);
		generateWattsStrogatz(mParams["numNodes"], mParams["radiusMean"], mParams["radiusStd"], mNumNeighbors, mRewireProb);
	}
	break;
	}
}