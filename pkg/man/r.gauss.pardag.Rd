\name{r.gauss.pardag}
\alias{r.gauss.pardag}
\encoding{UTF-8}
\concept{Gaussian causal model}
\concept{random graph}
\title{Generate a Gaussian Causal Model Randomly}
\description{
  Generate a random Gaussian causal model.  Parameters specifying the
  connectivity as well as coefficients and error terms of the corresponding
  linear structural equation model can be specified.  The observational
  expectation value of the generated model is always 0, meaning that no
  interception terms are drawn.
}
\usage{
r.gauss.pardag(p, prob, top.sort = FALSE, normalize = FALSE,
               lbe = 0.1, ube = 1, neg.coef = TRUE, labels = as.character(1:p),
               lbv = 0.5, ubv = 1)
}
\arguments{
  \item{p}{the number of nodes.}
  \item{prob}{probability of connecting a node to another node.}
  \item{top.sort}{\code{\link{logical}} indicating whether the output graph should be
    topologically sorted, meaning that arrows always point from lower
    to higher node indices.}
  \item{normalize}{\code{\link{logical}} indicating whether weights and error variances
    should be normalized such that the diagonal of the corresponding
    observational covariance matrix is 1.}
  \item{lbe, ube}{lower and upper bounds of the absolute values of edge
    weights.}
  \item{neg.coef}{logical indicating whether negative edge weights are also
    admissible.}
  \item{labels}{(optional) character vector of variable (or \dQuote{node}) names.}
  \item{lbv, ubv}{lower and upper bound on error variances of the noise terms
    in the structural equations.}
}
\details{
  The underlying directed acyclic
  graph (DAG) is generated by drawing an undirected graph from an Erdős-Rényi
  model orienting the edges according to a random topological ordering drawn
  uniformly from the set of permutations of \code{p} variables. This means that
  any two nodes are connected with (the same) probability \code{prob}, and that
  the connectivity of different pairs of nodes is independent.

  A Gaussian causal model can be represented as a set of linear structural
  equations.  The regression coefficients of the model can be represented as
  "edge weights" of the DAG.  Edge weights are drawn uniformly and
  independently from the interval between \code{lbe} and \code{ube}; if
  \code{neg.coef = TRUE}, their sign is flipped with probability 0.5.  Error
  variances are drawn uniformly and independently from the interval between
  \code{lbv} and \code{ubv}.

  If \code{normalize = TRUE}, the edge weights and error variances are
  normalized \emph{in the end} to ensure that the diagonal elements of the
  observational covariance matrix are all 1; the procedure used is described in
  Hauser and Bühlmann (2012).  Note that in this case the error variances and
  edge weights are no longer guaranteed to lie in the specified intervals
  \emph{after normalization}.
}
\value{
  An object of class \code{"\linkS4class{GaussParDAG}"}.
}
\references{
  P. Erdős and A. Rényi (1960).  On the evolution of random graphs.
  \emph{Publications of the Mathematical Institute of the Hungarian Academy of
  Sciences} \bold{5}, 17--61.

  A. Hauser and P. Bühlmann (2012).  Characterization and greedy learning of
  interventional Markov equivalence classes of directed acyclic graphs.
  \emph{Journal of Machine Learning Research} \bold{13}, 2409--2464.
}
\author{
  Alain Hauser (\email{alain.hauser@bfh.ch})
}
\seealso{
  \code{\linkS4class{GaussParDAG}}, \code{\link{randomDAG}}
}
\examples{
set.seed(307)

## Plot some random DAGs
if (require(Rgraphviz)) {
  ## Topologically sorted random DAG
  myDAG <- r.gauss.pardag(p = 10, prob = 0.2, top.sort = TRUE)
  plot(myDAG)

  ## Unsorted DAG
  myDAG <- r.gauss.pardag(p = 10, prob = 0.2, top.sort = FALSE)
  plot(myDAG)
}

## Without normalization, edge weigths and error variances lie within the
## specified borders
set.seed(307)
myDAG <- r.gauss.pardag(p = 10, prob = 0.4,
  lbe = 0.1, ube = 1, lbv = 0.5, ubv = 1.5, neg.coef = FALSE)
B <- myDAG$weight.mat()
V <- myDAG$err.var()
any((B > 0 & B < 0.1) | B > 1)
any(V < 0.5 | V > 1.5)

## After normalization, edge weights and error variances are not necessarily
## within the specified range, but the diagonal of the observational covariance
## matrix consists of ones only
set.seed(308)
myDAG <- r.gauss.pardag(p = 10, prob = 0.4, normalize = TRUE,
  lbe = 0.1, ube = 1, lbv = 0.5, ubv = 1.5, neg.coef = FALSE)
B <- myDAG$weight.mat()
V <- myDAG$err.var()
any((B > 0 & B < 0.1) | B > 1)
any(V < 0.5 | V > 1.5)
diag(myDAG$cov.mat())
}
\keyword{datagen}
\keyword{graphs}
