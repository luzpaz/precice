// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "NearestNeighborMappingTest.hpp"
#include "mapping/NearestNeighborMapping.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Vertex.hpp"
#include "mesh/Data.hpp"
#include "utils/Parallel.hpp"
#include "utils/Dimensions.hpp"

#include "tarch/tests/TestCaseFactory.h"
registerTest(precice::mapping::tests::NearestNeighborMappingTest)

namespace precice {
namespace mapping {
namespace tests {

tarch::logging::Log NearestNeighborMappingTest::
  _log ( "precice::mapping::tests::NearestNeighborMappingTest" );

NearestNeighborMappingTest:: NearestNeighborMappingTest()
:
  TestCase ( "precice::mapping::tests::NearestNeighborMappingTest" )
{}

void NearestNeighborMappingTest:: run()
{
  PRECICE_MASTER_ONLY {
    testMethod(testConsistentNonIncremental);
    testMethod(testConservativeNonIncremental);
  }
}

void NearestNeighborMappingTest:: testConsistentNonIncremental()
{
  preciceTrace("testConsistentNonIncremental()");
  using namespace mesh;
  using utils::Vector2D;
  int dimensions = 2;

  // Create mesh to map from
  PtrMesh inMesh(new Mesh("InMesh", dimensions, false));
  PtrData inData = inMesh->createData("InData", 1);
  int inDataID = inData->getID();
  Vertex& inVertex0 = inMesh->createVertex(Vector2D(0.0));
  Vertex& inVertex1 = inMesh->createVertex(Vector2D(1.0));
  inMesh->allocateDataValues();
  utils::DynVector& inValues = inData->values();
  inValues[0] = 1.0;
  inValues[1] = 2.0;

  // Create mesh to map to
  PtrMesh outMesh(new Mesh("OutMesh", dimensions, false));
  PtrData outData = outMesh->createData("OutData", 1);
  int outDataID = outData->getID();
  Vertex& outVertex0 = outMesh->createVertex(Vector2D(0.0));
  Vertex& outVertex1 = outMesh->createVertex(Vector2D(1.0));
  outMesh->allocateDataValues();

  // Setup mapping with mapping coordinates and geometry used
  NearestNeighborMapping mapping(Mapping::CONSISTENT);
  mapping.setMeshes(inMesh, outMesh);
  validateEquals(mapping.hasComputedMapping(), false);

  // Map data with coinciding vertices, has to result in equal values.
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  const utils::DynVector& outValues = outData->values();
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[0], inValues[0]);
  validateNumericalEquals(outValues[1], inValues[1]);

  // Map data with almost coinciding vertices, has to result in equal values.
  inVertex0.setCoords(outVertex0.getCoords() + Vector2D(0.1));
  inVertex1.setCoords(outVertex1.getCoords() + Vector2D(0.1));
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[0], inValues[0]);
  validateNumericalEquals(outValues[1], inValues[1]);

  // Map data with exchanged vertices, has to result in exchanged values.
  inVertex0.setCoords(outVertex1.getCoords());
  inVertex1.setCoords(outVertex0.getCoords());
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[1], inValues[0]);
  validateNumericalEquals(outValues[0], inValues[1]);

  // Map data with coinciding output vertices, has to result in same values.
  outVertex1.setCoords(outVertex0.getCoords());
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[1], inValues[1]);
  validateNumericalEquals(outValues[0], inValues[1]);
}

void NearestNeighborMappingTest:: testConservativeNonIncremental()
{
  preciceTrace("testConservativeNonIncremental()");
  using namespace mesh;
  using namespace tarch::la;
  using utils::Vector2D;
  int dimensions = 2;

  // Create mesh to map from
  PtrMesh inMesh(new Mesh("InMesh", dimensions, false));
  PtrData inData = inMesh->createData("InData", 1);
  int inDataID = inData->getID();
  Vertex& inVertex0 = inMesh->createVertex(Vector2D(0.0));
  Vertex& inVertex1 = inMesh->createVertex(Vector2D(1.0));
  inMesh->allocateDataValues();
  utils::DynVector& inValues = inData->values();
  inValues[0] = 1.0;
  inValues[1] = 2.0;

  // Create mesh to map to
  PtrMesh outMesh(new Mesh("OutMesh", dimensions, false));
  PtrData outData = outMesh->createData("OutData", 1);
  int outDataID = outData->getID();
  Vertex& outVertex0 = outMesh->createVertex(Vector2D(0.0));
  Vertex& outVertex1 = outMesh->createVertex(Vector2D(1.0));
  outMesh->allocateDataValues();

  // Setup mapping with mapping coordinates and geometry used
  NearestNeighborMapping mapping(Mapping::CONSERVATIVE);
  mapping.setMeshes(inMesh, outMesh);
  validateEquals(mapping.hasComputedMapping(), false);

  // Map data with coinciding vertices, has to result in equal values.
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  utils::DynVector& outValues = outData->values();
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[0], inValues[0]);
  validateNumericalEquals(outValues[1], inValues[1]);
  assign(outValues) = 0.0;

  // Map data with almost coinciding vertices, has to result in equal values.
  inVertex0.setCoords(outVertex0.getCoords() + Vector2D(0.1));
  inVertex1.setCoords(outVertex1.getCoords() + Vector2D(0.1));
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[0], inValues[0]);
  validateNumericalEquals(outValues[1], inValues[1]);
  assign(outValues) = 0.0;

  // Map data with exchanged vertices, has to result in exchanged values.
  inVertex0.setCoords(outVertex1.getCoords());
  inVertex1.setCoords(outVertex0.getCoords());
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[1], inValues[0]);
  validateNumericalEquals(outValues[0], inValues[1]);
  assign(outValues) = 0.0;

  // Map data with coinciding output vertices, has to result in double values.
  outVertex1.setCoords(Vector2D(-1.0));
  mapping.computeMapping();
  mapping.map(inDataID, outDataID);
  validateEquals(mapping.hasComputedMapping(), true);
  validateNumericalEquals(outValues[0], inValues[0] + inValues[1]);
  validateNumericalEquals(outValues[1], 0.0);
}

}}} // namespace precice, mapping, tests