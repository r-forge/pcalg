/**
 * Main file of the Greedy Interventional Equivalence Search library for R
 *
 * @author Alain Hauser
 * $Id$
 */

#include <Rcpp.h>
#include <vector>
#include <string>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/graph/adjacency_list.hpp>


// For Mac g++ compiler... that bastard doesn't know uint...
typedef unsigned int uint;

// Define BGL class for undirected graph
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> UndirectedGraph;

#include "score.hpp"
#include "greedy.hpp"
#include "constraint.hpp"
#define DEFINE_GLOBAL_DEBUG_STREAM
#include "gies_debug.hpp"

using namespace boost::lambda;


/**
 * Reads in a graph from a list of in-edges passed as a SEXP to
 * an EssentialGraph object
 */
EssentialGraph castGraph(SEXP argInEdges)
{
	int i;
	Rcpp::List listInEdges(argInEdges);
	std::vector<uint> vecParents;
	std::vector<uint>::iterator vi;
	EssentialGraph result(listInEdges.size());
	for (i = 0; i < listInEdges.size(); ++i) {
		vecParents = listInEdges[i];
		// Adapt indices to C++ convention
		for (vi = vecParents.begin(); vi != vecParents.end(); ++vi)
			result.addEdge(*vi - 1, i);
	}

	return result;
}

/**
 * Wrap a graph structure to an R list of in-edges
 */
Rcpp::List wrapGraph(EssentialGraph graph)
{
	Rcpp::List result;
	Rcpp::IntegerVector vecEdges;
	std::set<uint> edges;
	std::set<uint>::iterator si;
	int i;

	for (i = 0; i < graph.getVertexCount(); ++i) {
		edges = graph.getInEdges(i);
		vecEdges = Rcpp::IntegerVector();
		for (si = edges.begin(); si != edges.end(); ++si)
			vecEdges.push_back(*si + 1);
		result.push_back(vecEdges);
	}

	return result;
}

/**
 * Yields the local score of a vertex given its parents.
 *
 * @param	argScore		name of the score
 * @param	argPreprocData	preprocessed data; sufficient statistic and all
 * 							parameters characterizing the score to be calculated
 * @param 	argVertex		vertex index
 * @param 	argParents		vector of parents of vertex
 * @param	argOptions		additional options; at the moment: DEBUG.LEVEL
 * @return	local score value
 */
RcppExport SEXP localScore(
		SEXP argScore,
		SEXP argPreprocData,
		SEXP argVertex,
		SEXP argParents,
		SEXP argOptions)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Set debug level
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));
	dout.level(1) << "Calculating local score...\n";

	// Create appropriate scoring object
	Rcpp::List data(argPreprocData);
	TargetFamily targets = castTargets(data["targets"]);
	dout.level(3) << "# intervention targets: " << targets.size() << "\n";
	Score* score = createScore(Rcpp::as<std::string>(argScore), &targets, data);

	// Calculate local score and delete score object
	double result = score->local(Rcpp::as<uint>(argVertex) - 1, castVertices(argParents));
	delete score;
	return Rcpp::wrap(result);

	END_RCPP
}

/**
 * Yields the global score of a DAG.
 *
 * @param	argScore		name of the score
 * @param	argPreprocData	preprocessed data; sufficient statistic and all
 * 							parameters characterizing the score to be calculated
 * @param 	argInEdges		list of in-edges characterizing the DAG
 * @param	argOptions		additional options; at the moment: DEBUG.LEVEL
 * @return	global score value
 */
RcppExport SEXP globalScore(
		SEXP argScore,
		SEXP argPreprocData,
		SEXP argInEdges,
		SEXP argOptions)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Set debug level
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));

	// Create appropriate scoring object
	Rcpp::List data(argPreprocData);
	TargetFamily targets = castTargets(data["targets"]);
	Score* score = createScore(Rcpp::as<std::string>(argScore), &targets, data);

	// Calculate local score
	double result = score->global(castGraph(argInEdges));
	delete score;
	return Rcpp::wrap(result);

	END_RCPP
}

/**
 * Yields the local MLE of a vertex given its parents.
 *
 * @param	argScore		name of the score
 * @param	argPreprocData	preprocessed data; sufficient statistic and all
 * 							parameters characterizing the score to be calculated
 * @param 	argVertex		vertex index
 * @param 	argParents		vector of parents of vertex
 * @param	argOptions		additional options; at the moment: DEBUG.LEVEL
 * @return	vector of local MLE
 */
RcppExport SEXP localMLE(
		SEXP argScore,
		SEXP argPreprocData,
		SEXP argVertex,
		SEXP argParents,
		SEXP argOptions)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Set debug level
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));

	// Create appropriate scoring object
	Rcpp::List data(argPreprocData);
	TargetFamily targets = castTargets(data["targets"]);
	Score* score = createScore(Rcpp::as<std::string>(argScore), &targets, data);

	// Calculate local score
	std::vector<double> result = score->localMLE(Rcpp::as<uint>(argVertex) - 1, castVertices(argParents));
	delete score;
	return Rcpp::wrap(result);

	END_RCPP
}

/**
 * Yields the global MLE of a DAG.
 *
 * @param	argScore		name of the score
 * @param	argPreprocData	preprocessed data; sufficient statistic and all
 * 							parameters characterizing the score to be calculated
 * @param 	argInEdges		list of in-edges characterizing the DAG
 * @param	argOptions		additional options; at the moment: DEBUG.LEVEL
 * @return	list of MLE vectors
 */
RcppExport SEXP globalMLE(
		SEXP argScore,
		SEXP argPreprocData,
		SEXP argInEdges,
		SEXP argOptions)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Set debug level
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));

	// Create appropriate scoring object
	Rcpp::List data(argPreprocData);
	TargetFamily targets = castTargets(data["targets"]);
	Score* score = createScore(Rcpp::as<std::string>(argScore), &targets, data);

	// Calculate local score
	std::vector<std::vector<double> > result = score->globalMLE(castGraph(argInEdges));
	delete score;
	return Rcpp::wrap(result);

	END_RCPP
}

/**
 * Interface to a variety of causal inference algorithms.
 *
 * @param 	argGraph			list of in-edges representing the current
 * 								essential graph
 * @param	argPreprocData		preprocessed data; sufficient statistic and all
 * 								parameters characterizing the score to be calculated
 * @param	argAlgorithm		string indicating the causal inference algorithm
 * 								to be used. Supported options: "GIES", "GDS", "DP"
 * @param	argScore			name of score object to be used. Currently supported:
 * 								"none" (= R object), "gauss.l0pen"
 * @param	argOptions			list of options specific for desired inference algorithm
 */
RcppExport SEXP causalInference(
		SEXP argGraph,
		SEXP argPreprocData,
		SEXP argAlgorithm,
		SEXP argScore,
		SEXP argOptions)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Cast debug level from options
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));

	// Cast graph
	dout.level(1) << "Casting graph...\n";
	EssentialGraph graph = castGraph(argGraph);

	// Cast list of targets
	dout.level(1) << "Casting list of targets...\n";
	Rcpp::List data(argPreprocData);
	TargetFamily targets = castTargets(data["targets"]);

	// Cast algorithm string
	dout.level(1) << "Casting algorithm and options...\n";
	std::string algName = Rcpp::as<std::string>(argAlgorithm);

	// TODO: cast score type, allow for C++ scoring objects
	// Up to now, only R functions are allowed for scoring...
	dout.level(1) << "Casting score...\n";
	Score* score = createScore(Rcpp::as<std::string>(argScore), &targets, data);

	graph.setScore(score);
	graph.setTargets(&targets);

	std::vector<int> steps;

	// Cast option for limits in vertex degree
	dout.level(1) << "Casting maximum vertex degree...\n";
	Rcpp::NumericVector maxDegree = options["maxdegree"];
	if (maxDegree.size() > 0) {
		if (maxDegree.size() == 1) {
			if (maxDegree[0] >= 1.) {
				uint uniformMaxDegree = static_cast<uint>(maxDegree[0]);
				graph.limitVertexDegree(uniformMaxDegree);
			}
			else {
				double maxRelativeDegree = maxDegree[0];
				graph.limitVertexDegree(maxRelativeDegree);
			}
		}
		else {
			std::vector<uint> maxDegrees = Rcpp::as< std::vector<uint> >(options["maxdegree"]);
			graph.limitVertexDegree(maxDegrees);
		}
	}
	
	// Cast option for vertices which are not allowed to have parents
	std::vector<uint> childrenOnly = Rcpp::as< std::vector<uint> >(options["childrenonly"]);
	std::for_each(childrenOnly.begin(), childrenOnly.end(), bind(&EssentialGraph::setChildrenOnly, &graph, _1, true));
	int stepLimit;

	// Perform inference algorithm:
	// GIES
	if (algName == "GIES") {
		dout.level(1) << "Performing GIES...\n";

		// Enable caching, if requested
		if (options["caching"])
			graph.enableCaching();

		// Perform a greedy search, with or without turning phase
		// TODO: evtl. zusätzlichen Parameter einfügen, der wiederholtes Suchen
		// auch ohne Drehphase erlaubt...
		if (options["turning"]) {
			bool cont;
			do {
				cont = false;
				for (steps.push_back(0); graph.greedyForward(); steps.back()++);
				for (steps.push_back(0); graph.greedyBackward(); steps.back()++)
					cont = true;
				for (steps.push_back(0); graph.greedyTurn(); steps.back()++)
					cont = true;
			} while (cont);
		}
		else {
			for (steps.push_back(0); graph.greedyForward(); steps.back()++);
			for (steps.push_back(0); graph.greedyBackward(); steps.back()++);
		}
	}

	// Single phase or step of GIES
	else if (algName == "GIES-F" || algName == "GIES-B" || algName == "GIES-T") {
		dout.level(1) << "Performing " << algName << "...\n";

		// Limit to single step if requested
		stepLimit = options["maxsteps"];
		if (stepLimit == 0)
			stepLimit = graph.getVertexCount()*graph.getVertexCount();

		// Enable caching, if requested
		if (options["caching"])
			graph.enableCaching();

		steps.push_back(0);
		if (algName == "GIES-F")
			for (; steps.back() < stepLimit && graph.greedyForward(); steps.back()++);
		else if (algName == "GIES-B")
			for (; steps.back() < stepLimit && graph.greedyBackward(); steps.back()++);
		else if (algName == "GIES-T")
			for (; steps.back() < stepLimit && graph.greedyTurn(); steps.back()++);
	}

	// Single one or several steps of GIES into either direction
	else if (algName == "GIES-STEP") {
		dout.level(1) << "Performing " << algName << "...\n";

		// Limit to single step if requested
		stepLimit = options["maxsteps"];
		if (stepLimit == 0)
			stepLimit = graph.getVertexCount()*graph.getVertexCount();

		// TODO: evtl. steps so ändern, dass man daraus ablesen kann, in welcher
		// Reihenfolge die einzelnen Phasen ausgeführt wurden
		// Steps: 3 entries, storing number of forward, backward, and turning steps
		steps.resize(3, 0);
		step_dir dir = SD_NONE;
		do {
			dir = graph.greedyStep();
			if (dir != SD_NONE)
				steps[dir - 1]++;
		} while (steps[0] + steps[1] + steps[2] < stepLimit && dir != SD_NONE);
	}

	// GDS
	else if (algName == "GDS") {
		// TODO: evtl. caching für GDS implementieren...
		// Perform a greedy search, with or without turning phase
		if (options["turning"]) {
			bool cont;
			do {
				cont = false;
				for (steps.push_back(0); graph.greedyDAGForward(); steps.back()++);
				for (steps.push_back(0); graph.greedyDAGBackward(); steps.back()++)
					cont = true;
				for (steps.push_back(0); graph.greedyDAGTurn(); steps.back()++)
					cont = true;
			} while (cont);
		}
		else {
			for (steps.push_back(0); graph.greedyDAGForward(); steps.back()++);
			for (steps.push_back(0); graph.greedyDAGBackward(); steps.back()++);
		}

		// Construct equivalence class
		graph.replaceUnprotected();
	}
	// DP
	else if (algName == "SiMy") {
		graph.siMySearch();
		graph.replaceUnprotected();
	}

	// Return new list of in-edges and steps
	delete score;
	return Rcpp::List::create(Rcpp::Named("in.edges") = wrapGraph(graph),
			Rcpp::Named("steps") = steps);

	END_RCPP
}

RcppExport SEXP representative(SEXP argGraph)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Cast graph
	EssentialGraph graph = castGraph(argGraph);

	// Get and return representative
	return wrapGraph(graph.getRepresentative());

	END_RCPP
}

RcppExport SEXP dagToEssentialGraph(SEXP argGraph, SEXP argTargets)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Cast arguments
	EssentialGraph graph = castGraph(argGraph);
	TargetFamily targets = castTargets(argTargets);

	// Calculate essential graph
	graph.setTargets(&targets);
	graph.replaceUnprotected();

	// Return essential graph
	return wrapGraph(graph);

	END_RCPP
}

RcppExport SEXP optimalTarget(SEXP argGraph, SEXP argMaxSize)
{
	// Initialize automatic exception handling; manual one does not work any more...
	BEGIN_RCPP

	// Cast arguments
	EssentialGraph graph = castGraph(argGraph);
	int maxSize = Rcpp::as<int>(argMaxSize);

	// Calculate optimal intervention target
	std::set<uint> target = graph.getOptimalTarget(maxSize);

	// Adapt numbering convention...
	std::vector<uint> result(target.begin(), target.end());
	std::for_each(result.begin(), result.end(), _1++);
	return Rcpp::wrap(result);

	END_RCPP
}

/**
 * Calculate the p-value for a conditional independence test on Gaussian data
 */
RcppExport SEXP condIndTestGauss(
		SEXP argVertex1,
		SEXP argVertex2,
		SEXP argCondSet,
		SEXP argSampleSize,
		SEXP argCor)
{
	// Exception handling
	BEGIN_RCPP

	int i;

	// Cast arguments; note index shift between R and C++!
	uint u = Rcpp::as<uint>(argVertex1) - 1;
	uint v = Rcpp::as<uint>(argVertex2) - 1;
	std::vector<uint> S = Rcpp::as<std::vector<uint> >(argCondSet);
	for (i = 0; i < S.size(); ++i) S[i]--;
	uint n = Rcpp::as<uint>(argSampleSize);
	Rcpp::NumericMatrix cor(argCor);

	// Create test object and calculate p-value
	IndepTestGauss indepTest(n, cor);
	return Rcpp::wrap(indepTest.test(u, v, S));

	END_RCPP
}


/**
 * Perform undirected version of PC algorithm, i.e., estimate skeleton of DAG
 * given data
 *
 */
RcppExport SEXP estimateSkeleton(
		SEXP argAdjMatrix,
		SEXP argSuffStat,
		SEXP argIndepTest,
		SEXP argAlpha,
		SEXP argOptions)
{
	// Exception handling
	BEGIN_RCPP

	// Cast debug level from options
	Rcpp::List options(argOptions);
	dout.setLevel(Rcpp::as<int>(options["DEBUG.LEVEL"]));

	int i, j;

	dout.level(1) << "Casting arguments...\n";

	// Cast sufficient statistic and significance level
	dout.level(2) << "Casting sufficient statistic...\n";
	double alpha = Rcpp::as<double>(argAlpha);
	Rcpp::List suffStat(argSuffStat);
	Rcpp::NumericMatrix cor((SEXP)(suffStat["C"]));

	// Cast independence test
	dout.level(2) << "Casting independence test...\n";
	// TODO cast indep test
	IndepTestGauss indepTest(Rcpp::as<uint>(suffStat["n"]), cor);

	// Cast graph
	dout.level(2) << "Casting graph...\n";
	Rcpp::LogicalMatrix adjMatrix(argAdjMatrix);
	int p = adjMatrix.nrow();
	Skeleton graph(p);
	std::vector<uint> emptySet;
	double pval;
	for (i = 0; i < p; i++)
		for (j = i + 1; j < p; j++)
			if (adjMatrix(i, j)) {
				pval = indepTest.test(i, j, emptySet);
				dout.level(2) << "  x = " << i << ", y = " << j << ", S = () : pval = " << pval << std::endl;
				if (pval < alpha) graph.addEdge(i, j);
			}

	// Create test object and estimate skeleton
	graph.setIndepTest(&indepTest);
	dout.level(1) << "Fitting skeleton to data...\n";
	graph.fitCondInd(alpha);

	// Return adjacency matrix
	return graph.getAdjacencyMatrix();

	END_RCPP
}

