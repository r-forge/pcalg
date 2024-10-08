\name{gmI}
\title{Graphical Model 7-dim IDA Data Examples}
\alias{gmI}
\alias{gmI7}
\docType{data}
\description{
  This data set contains a matrix containing information on seven gaussian
  variables and the corresonding DAG model.
}
\usage{data(gmI)}
\format{
  The two \code{gmI*} objects are each a \code{\link{list}} of two components
  \code{x}, an \eqn{n \times 7}{n * 7} numeric matrix, and
  \code{g}, a DAG, a graph generated by \code{\link{randomDAG}}.

  See \code{\link{gmG}} for more
}
\details{
  The data was generated as indicated below.  First, a random DAG was
  generated, then samples were drawn from this model, strictly
  speaking for \code{gmI7} only.
}
\source{
  The data sets are \code{\link{identical}} to those generated by
 \preformatted{
   ## Used to generate "gmI"
   set.seed(123)
   p <- 7
   myDAG <- randomDAG(p, prob = 0.2) ## true DAG
   gmI  <- list(x = rmvDAG(10000, myDAG, back.compatible=TRUE), g = myDAG)
   gmI7 <- list(x = rmvDAG( 8000, myDAG),                       g = myDAG)
 }
}
\examples{
data(gmI)
str(gmI, max=3)
stopifnot(identical(gmI $ g, gmI7 $ g))
if(dev.interactive()) { ## to save time in tests
  round(as(gmI $ g, "Matrix"), 2) # weight ("adjacency") matrix
  if (require(Rgraphviz)) plot(gmI $ g)
  pairs(gmI$x, gap = 0,
        panel=function(...) smoothScatter(..., add=TRUE))
}
}
\keyword{datasets}
