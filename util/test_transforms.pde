// Test Processings transformation behavior
float radius = 100;
float zv = TWO_PI/20;
float ro = TWO_PI/5;

void setup() {
  size(800, 800, P3D);
  
  // Run tests
  testRotatedTranslation();
  testSide1Sequence();
  testMiddleFaceRotations();
  testCompleteMiddleFaceSequence();
  testAllSideTransforms();
  
  // Exit after tests
  exit();
}

void draw() {
  background(0);
  
  println("=== Basic Transformation Tests ===");
  
  // Test 1: Single rotateX(PI/2)
  pushMatrix();
  println("\nTest rotateX(PI/2):");
  println("Before:");
  printMatrix(getMatrix().get(null));
  
  rotateX(PI/2);
  println("\nAfter rotateX(PI/2):");
  println("Point(1,0,0):", modelX(1,0,0), modelY(1,0,0), modelZ(1,0,0));
  println("Point(0,1,0):", modelX(0,1,0), modelY(0,1,0), modelZ(0,1,0));
  println("Point(0,0,1):", modelX(0,0,1), modelY(0,0,1), modelZ(0,0,1));
  printMatrix(getMatrix().get(null));
  popMatrix();
  
  // Test 2: Single rotateZ(PI/2)
  pushMatrix();
  println("\nTest rotateZ(PI/2):");
  println("Before:");
  printMatrix(getMatrix().get(null));
  
  rotateZ(PI/2);
  println("\nAfter rotateZ(PI/2):");
  println("Point(1,0,0):", modelX(1,0,0), modelY(1,0,0), modelZ(1,0,0));
  println("Point(0,1,0):", modelX(0,1,0), modelY(0,1,0), modelZ(0,1,0));
  println("Point(0,0,1):", modelX(0,0,1), modelY(0,0,1), modelZ(0,0,1));
  printMatrix(getMatrix().get(null));
  popMatrix();
  
  // Test 3: Single rotateX(PI)
  pushMatrix();
  println("\nTest rotateX(PI):");
  println("Before:");
  printMatrix(getMatrix().get(null));
  
  rotateX(PI);
  println("\nAfter rotateX(PI):");
  println("Point(1,0,0):", modelX(1,0,0), modelY(1,0,0), modelZ(1,0,0));
  println("Point(0,1,0):", modelX(0,1,0), modelY(0,1,0), modelZ(0,1,0));
  println("Point(0,0,1):", modelX(0,0,1), modelY(0,0,1), modelZ(0,0,1));
  printMatrix(getMatrix().get(null));
  popMatrix();
}

void printMatrix(float[] m) {
  for (int i = 0; i < 4; i++) {
    print("[ ");
    for (int j = 0; j < 4; j++) {
      print(nf(m[j*4 + i], 0, 2) + " ");
    }
    println("]");
  }
}

void testMatrixStack() {
  pushMatrix();
  
  println("\nTesting matrix stack...");
  
  // Initial state
  println("\nInitial basis vectors:");
  printBasisVectors();
  
  // Test sequence
  translate(1, 0, 0);
  pushMatrix();
  rotateX(PI/2);
  
  println("\nAfter translate(1,0,0) and rotateX(PI/2):");
  printBasisVectors();
  
  // Test specific point [0,0,1]
  float[] p = {0, 0, 1};
  float[] result = getTransformedPoint(p);
  println(String.format("\nPoint (0,0,1) transformed to: (%.1f,%.1f,%.1f)", 
          result[0], result[1], result[2]));
  
  printMatrix(getMatrix().get(null));
  
  popMatrix();
}

void printBasisVectors() {
  float[] x = getTransformedPoint(new float[]{1,0,0});
  float[] y = getTransformedPoint(new float[]{0,1,0});
  float[] z = getTransformedPoint(new float[]{0,0,1});
  println("Basis X:", x[0], x[1], x[2]);
  println("Basis Y:", y[0], y[1], y[2]);
  println("Basis Z:", z[0], z[1], z[2]);
}

float[] getTransformedPoint(float[] p) {
  float[] result = new float[3];
  result[0] = modelX(p[0], p[1], p[2]);
  result[1] = modelY(p[0], p[1], p[2]);
  result[2] = modelZ(p[0], p[1], p[2]);
  return result;
}

void testRotatedTranslation() {
  pushMatrix();
  
  println("\nTesting translation along rotated axes...");
  
  // Initial state
  println("\nInitial basis vectors:");
  printBasisVectors();
  
  // Test sequence
  rotateX(PI/4);  // 45° around X
  translate(0, 0, 10);  // Translate along rotated Z
  
  println("\nAfter rotateX(PI/4) and translate(0,0,10):");
  printBasisVectors();
  
  // Test origin transformation
  float[] p = {0, 0, 0};
  float[] result = getTransformedPoint(p);
  println(String.format("\nPoint (0,0,0) transformed to: (%.1f,%.1f,%.1f)", 
          result[0], result[1], result[2]));
  
  printMatrix(getMatrix().get(null));
  
  popMatrix();
}

void testSide1Sequence() {
  pushMatrix();
  
  println("\nTesting Side 1 sequence...");
  
  // Initial point
  float x = 1.49;
  float y = -0.02;
  float[] point = {x, y, 0};
  println(String.format("Initial point: (%.3f, %.3f, %.0f)", point[0], point[1], point[2]));
  
  // 1. LED space rotation (PI/10)
  rotateZ(PI/10);
  println("\nAfter LED space rotation:");
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // 2. Initial transform
  resetMatrix();
  rotateX(PI);  // Flip upside down
  rotateZ(-3 * TWO_PI/5);  // Side 1 rotation
  println("\nAfter initial transform:");
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // 3. Side positioning
  resetMatrix();
  rotateX(PI);  // Flip upside down
  rotateZ(TWO_PI/20);  // Side 1 positioning (zv)
  rotateX(1.1071);  // Side tilt (xv)
  println("\nAfter side positioning:");
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  popMatrix();
}

void testMiddleFaceRotations() {
  pushMatrix();
  
  println("\nTesting middle face rotations...");
  float[] point = {0, 100, 0};  // Point on face
  
  // Test bottom half face (1-5)
  println("\nBottom half face (1-5):");
  rotateX(PI);
  rotateZ(ro*1 + zv - ro);  // Side 1
  rotateX(1.1071);  // xv
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // Test top half face (6-10)
  resetMatrix();
  println("\nTop half face (6-10):");
  rotateX(PI);
  rotateZ(ro*6 - zv + ro*3);  // Side 6
  rotateX(PI - 1.1071);  // PI - xv
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
          
  popMatrix();
}

void testCompleteMiddleFaceSequence() {
  pushMatrix();
  
  println("\nTesting complete middle face sequence...");
  float[] point = {1.49, -0.02, 0};  // First LED position
  
  // Initial LED space rotation (72°)
  println("\nAfter LED space rotation (-72°):");
  rotateZ(-PI/5);  // -72° = -PI/2.5 = -PI/5
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // Complete sequence
  resetMatrix();
  rotateX(PI);
  rotateZ(ro);
  rotateZ(ro*1 + zv - ro);
  rotateX(1.1071);  // xv
  translate(0, 0, radius*1.34);
  println("\nAfter complete sequence:");
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
          
  popMatrix();
}

void testAllSideTransforms() {
  pushMatrix();
  
  println("\nTesting all side transforms...");
  float[] point = {1.49, -0.02, 0};  // LED 1 position
  
  // Test top face (side 0)
  println("\nSide 0 (top):");
  rotateX(PI);
  rotateZ(ro*0);  // No extra rotation
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // Test middle face (side 5)
  resetMatrix();
  println("\nSide 5 (middle):");
  rotateX(PI);
  rotateZ(ro*5 + zv - ro);
  rotateX(1.1071);  // xv
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
  
  // Test bottom face (side 11)
  resetMatrix();
  println("\nSide 11 (bottom):");
  rotateX(PI);
  rotateZ(ro*11);
  println(String.format("Point: (%.3f, %.3f, %.3f)", 
          modelX(point[0], point[1], point[2]),
          modelY(point[0], point[1], point[2]),
          modelZ(point[0], point[1], point[2])));
          
  popMatrix();
} 