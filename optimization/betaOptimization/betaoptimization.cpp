#include "betaoptimization.h"

using namespace Eigen;
using namespace std;

//Sequential Quadratic Programming(local and derivative-based)
#define USED_ALGO NLOPT_LD_SLSQP

#define BOTTOM_UP_START_DEPTH 3

#define IGNORE_BINARIES true

#define s_1  S(0)
#define s_x  S(1)
#define s_y  S(2)
#define s_z  S(3)
#define s_xy  S(4)
#define s_yz  S(5)
#define s_xz  S(6)
#define s_x2  S(7)
#define s_y2  S(8)
#define s_z2  S(9)

#define Smat_1(i) BetaOptimization::S_mat(0,i)
#define Smat_x(i) BetaOptimization::S_mat(1,i)
#define Smat_y(i) BetaOptimization::S_mat(2,i)
#define Smat_z(i) BetaOptimization::S_mat(3,i)
#define Smat_xy(i) BetaOptimization::S_mat(4,i)
#define Smat_yz(i) BetaOptimization::S_mat(5,i)
#define Smat_xz(i) BetaOptimization::S_mat(6,i)
#define Smat_x2(i) BetaOptimization::S_mat(7,i)
#define Smat_y2(i) BetaOptimization::S_mat(8,i)
#define Smat_z2(i) BetaOptimization::S_mat(9,i)

#define MAX(v1,v2) (v1>v2?v1:v2);
#define MIN(v1,v2) (v1<v2?v1:v2);

#define GAMMA_C_TOP (1e-4)
#define GAMMA_I_TOP (1e-0)
#define GAMMA_L_TOP (1e-1)

#define GAMMA_I_TIPPE_TOP (1e-0)
#define GAMMA_L_TIPPE_TOP (1e-0)

#define GAMMA_I_YOYO (1e-0)
#define GAMMA_L_YOYO (1e-0)

#define MAX_TIME (30*60)

#define ONLY_LOWERING false

#define NUMBER_IS_ACCEPTABLE (number<=MAX_NUMBER_OF_VARIABLES)

#define MAX_NUMBER_OF_VARIABLES 3800

#define FIXED_EPSILON (0.5f*1e-1)

#define OPTIMIZATION_FUNCTION_THRESHOLD (1e-4)
#define OPTIMIZATION_CONSTAINTS_THRESHOLD (1e-8)

#define IS_TOP_OPTIMIZATION optimization_type == OPTIMIZATION_TYPE_TOP
#define IS_TIPPE_TOP_OPTIMIZATION optimization_type == OPTIMIZATION_TYPE_TIPPE_TOP

/* weights for optimization */

double BetaOptimization::gamma_c_top = GAMMA_C_TOP;
double BetaOptimization::gamma_i_top = GAMMA_I_TOP;
double BetaOptimization::gamma_l_top = GAMMA_L_TOP;

double BetaOptimization::gamma_i_tippe_top = GAMMA_I_TIPPE_TOP;
double BetaOptimization::gamma_l_tippe_top = GAMMA_L_TIPPE_TOP;

double BetaOptimization::gamma_i_yoyo = GAMMA_I_YOYO;
double BetaOptimization::gamma_l_yoyo = GAMMA_L_YOYO;

Mesh* BetaOptimization::mesh = NULL;
ExtendedOctree BetaOptimization::octree;
Mesh* BetaOptimization::shellMesh = NULL;
VectorXd BetaOptimization::S_comp;
MatrixXd BetaOptimization::S_mat;
SpMat BetaOptimization::L;

GLfloat BetaOptimization::align_phi = 0.f;
GLfloat BetaOptimization::phi = 0.f;
float BetaOptimization::mesh_volume[10];

QVector3D BetaOptimization::com1;
QVector3D BetaOptimization::com2;

VectorXd BetaOptimization::S_inner_comp;

bool* BetaOptimization::isUsed;
double* BetaOptimization::currentBetas;
int BetaOptimization::betaSize;

/**
 * @brief BetaOptimization::doTopOptimization Execute the optimization for a top.
 */
void BetaOptimization::doTopOptimization()
{

    cout << "----------------------------------------" << "----------------------------------------" << endl;
    cout << "Top Optimization (Start)" << endl;


    BetaOptimization::mesh->swapYZ();

    BetaOptimization::setSForCompleteMesh();
    BetaOptimization::setCheckMatrixForCubes();
    BetaOptimization::octree.deleteNodeMeshes();
    BetaOptimization::resetPhi();

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Start Inertia Properties" << endl;
    showProperties(S_comp,S_mat,NULL,BetaOptimization::phi);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::optimizeBetasBottomUp(OPTIMIZATION_TYPE_TOP);
    BetaOptimization::optimizeBetasWithSplitAndMerge(OPTIMIZATION_TYPE_TOP);
    //BetaOptimization::optimizeBetas(OPTIMIZATION_TYPE_TOP);

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Final Inertia Properties" << endl;
    QVector<octree::cubeObject>* cubeVector = NULL;
    cubeVector = BetaOptimization::octree.getMergedCubes();
    BetaOptimization::setSMatrixForCubes(cubeVector);
    BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi,true);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::finishBetaOptimization();

    BetaOptimization::mesh->swapYZ();


    cout << "----------------------------------------" << endl;
    cout << "Top Optimization (Finish)" << endl;

}

/**
 * @brief BetaOptimization::doTippeTopOptimization Execute the optimization for a tippe top.
 */
void BetaOptimization::doTippeTopOptimization()
{

    cout << "----------------------------------------" << "----------------------------------------" << endl;
    cout << "Tippe Top Optimization (Start)" << endl;

    BetaOptimization::mesh->swapYZ();

    BetaOptimization::setSForCompleteMesh();
    BetaOptimization::setCheckMatrixForCubes();
    BetaOptimization::octree.deleteNodeMeshes();
    BetaOptimization::resetPhi();

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Start Inertia Properties" << endl;
    showProperties(S_comp,S_mat,NULL,BetaOptimization::phi);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::optimizeBetasBottomUp(OPTIMIZATION_TYPE_TIPPE_TOP);
    BetaOptimization::optimizeBetasWithSplitAndMerge(OPTIMIZATION_TYPE_TIPPE_TOP);
    //BetaOptimization::optimizeBetas(OPTIMIZATION_TYPE_TIPPE_TOP);

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Final Inertia Properties" << endl;
    QVector<octree::cubeObject>* cubeVector = NULL;
    cubeVector = BetaOptimization::octree.getMergedCubes();
    BetaOptimization::setSMatrixForCubes(cubeVector);
    BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi,true);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::finishBetaOptimization();

    BetaOptimization::mesh->swapYZ();

    cout << "----------------------------------------" << endl;
    cout << "Tippe Top Optimization (Finish)" << endl;

}

/**
 * @brief BetaOptimization::doYoyoOptimization Execute the optimization for a yoyo.
 */
void BetaOptimization::doYoyoOptimization()
{

    cout << "----------------------------------------" << "----------------------------------------" << endl;
    cout << "Yoyo Optimization (Start)" << endl;

    BetaOptimization::mesh->swapYZ();

    BetaOptimization::setSForCompleteMesh();
    BetaOptimization::setCheckMatrixForCubes();
    BetaOptimization::octree.deleteNodeMeshes();
    BetaOptimization::resetPhi();

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Start Inertia Properties" << endl;
    showProperties(S_comp,S_mat,NULL,BetaOptimization::phi);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::optimizeBetasBottomUp(OPTIMIZATION_TYPE_YOYO);
    BetaOptimization::optimizeBetasWithSplitAndMerge(OPTIMIZATION_TYPE_YOYO);
    //BetaOptimization::optimizeBetas(OPTIMIZATION_TYPE_TOP);

    /* -------------------------------------------------------------------------- */
    cout << "----------------------------------------" << endl;
    cout << "Final Inertia Properties" << endl;
    QVector<octree::cubeObject>* cubeVector = NULL;
    cubeVector = BetaOptimization::octree.getMergedCubes();
    BetaOptimization::setSMatrixForCubes(cubeVector);
    BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi,true);
    /* -------------------------------------------------------------------------- */

    BetaOptimization::finishBetaOptimization();

    BetaOptimization::mesh->swapYZ();

    cout << "----------------------------------------" << endl;
    cout << "Yoyo Optimization (Finish)" << endl;

}

/**
 * @brief BetaOptimization::initializeOctree Initialize the octree for the optimation.
 * @param newModifiedMesh the mesh for the optimation
 * @param startDepth the start depth
 * @param maximumDepth the maximum depth
 * @param shellExtensionValue the shell extension value
 */
void BetaOptimization::initializeOctree(
        Mesh* newModifiedMesh,
        GLint startDepth,
        GLint maximumDepth,
        GLint shellExtensionValue)
{

    if(BetaOptimization::mesh != NULL)
    {
        delete BetaOptimization::mesh;
    }

    cout << "----------------------------------------" << "----------------------------------------" << endl;
    cout << "Create Octree (Start)" << endl;

    BetaOptimization::mesh = newModifiedMesh;

    /*--------------------------------------------------------------*/
    // pre processing for align octree according to center of mass and x and y inertia axes
    BetaOptimization::mesh->swapYZ();

    QMatrix4x4 mat;
    VectorXd S;

    BetaOptimization::setSForCompleteMesh();
    S = BetaOptimization::S_comp;

    QVector3D vec1(s_x,s_y,0);
    QVector3D vec2(s_x,0,s_y);
    BetaOptimization::com1 = vec1/s_1;
    BetaOptimization::com2 = vec2/s_1;

    com1 *=-1;

    mat.setToIdentity();
    mat.translate(com1);

    BetaOptimization::mesh->transform(mat);

    BetaOptimization::setSForCompleteMesh();
    S = BetaOptimization::S_comp;

    double phi = BetaOptimization::executePhiOptimization();
    phi = (phi*180)/PI;

    cout << "----------------------------------------" << endl;
    cout << "Aligned phi is " << phi << "!" << endl;

    mat.setToIdentity();
    mat.rotate(phi,QVector3D(0,0,1));
    BetaOptimization::mesh->transform(mat);

    BetaOptimization::align_phi = phi;

    BetaOptimization::mesh->swapYZ();

    mat.setToIdentity();
    mat.rotate(-phi,QVector3D(0,1,0));
    com2 = mat*com2;
    /*--------------------------------------------------------------*/

    // initialize the octree
    // set point cloud
    BetaOptimization::octree.setMesh(BetaOptimization::mesh);
    BetaOptimization::octree.setStartMaxDepth(startDepth);
    BetaOptimization::octree.setOptimizationMaxDepth(maximumDepth);
    BetaOptimization::octree.quantizeSurface();
    BetaOptimization::octree.setupVectors();

    // calculate outer and inner cells
    BetaOptimization::octree.setupOctree();
    BetaOptimization::octree.setupOuterNodes();
    BetaOptimization::octree.setupInnerNodes();

    BetaOptimization::octree.printNumberOfLeafTypes();

    BetaOptimization::octree.adjustToBasicMaxDepth();

    BetaOptimization::octree.increaseShell(shellExtensionValue);

    BetaOptimization::octree.initiateMergeRoots();

    BetaOptimization::octree.makeDirty();

    // get mesh for inner shell (needed for view)
    BetaOptimization::octree.createInnerSurface();


    Mesh* newShellMesh = BetaOptimization::getShellMesh();
    if(BetaOptimization::shellMesh != NULL)
    {
       delete BetaOptimization::shellMesh;
    }
    BetaOptimization::shellMesh = newShellMesh;

    /*--------------------------------------------------------------*/
    mat.setToIdentity();
    mat.translate(com2);
    BetaOptimization::mesh->transform(mat);
    BetaOptimization::octree.transformOctree(mat);
    /*--------------------------------------------------------------*/

    BetaOptimization::octree.resetBetaOctreeState();

    cout << "----------------------------------------" << endl;
    cout << "Create Octree (Finish)" << endl;

}

/**
 * @brief spinItContraints Constraints function for spin it optimization!
 * @param n dimension
 * @param x current value
 * @param grad gradient
 * @param data additional parameter for choosing constraint
 * @return new value
 */
double BetaOptimization::spinItContraints(unsigned n, const double *x, double *grad, void *data)
{

   (void)n;

   spin_it_constraint_data *d = (spin_it_constraint_data *) data;
   int index = d->index;

   VectorXd betas(BetaOptimization::betaSize);
   GLdouble phi = x[0];

   int counter = 1;
   for(int i=0;i<BetaOptimization::betaSize;i++)
   {
       if(BetaOptimization::isUsed[i])
       {
           betas(i) = x[counter];
           counter++;
       }
       else
       {
           betas(i) = BetaOptimization::currentBetas[i];
       }
   }

   MatrixXd S = BetaOptimization::S_comp - BetaOptimization::S_mat*betas;

   double cosp = cos(phi);
   double sinp = sin(phi);

   switch(index)
   {
   case 0:

       // gradient for s_x constraint
       if(grad != NULL)
       {
           grad[0] = 0;
           //----------------------------------------
           int counter = 1;
           for(int i=0;i<BetaOptimization::betaSize;i++){

               if(BetaOptimization::isUsed[i])
               {
                   grad[counter] = -Smat_x(i);
                   counter++;
               }

           }
       }

       return s_x;
   case 1:

       // gradient for s_y constraint
       if(grad != NULL)
       {
          grad[0] = 0;
          //----------------------------------------
          int counter = 1;
          for(int i=0;i<BetaOptimization::betaSize;i++){

              if(BetaOptimization::isUsed[i])
              {
                  grad[counter] = -Smat_y(i);
                  counter++;
              }

          }

       }

       return s_y;
   case 2:

       // gradient for s_yz constraint
       if(grad != NULL)
       {
          grad[0] = 0;
          //----------------------------------------
          int counter = 1;
          for(int i=0;i<BetaOptimization::betaSize;i++){

              if(BetaOptimization::isUsed[i])
              {
                  grad[counter] = -Smat_yz(i);
                  counter++;
              }

          }

       }

       return s_yz;
   case 3:

       // gradient for s_xz constraint
       if(grad != NULL)
       {
          grad[0] = 0;
          //----------------------------------------
          int counter = 1;
          for(int i=0;i<BetaOptimization::betaSize;i++){

              if(BetaOptimization::isUsed[i])
              {
                  grad[counter] = -Smat_xz(i);
                  counter++;
              }

          }

       }

       return s_xz;
   case 4:

       // gradient for s_z constraint
       if(grad != NULL)
       {
          grad[0] = 0;
          //----------------------------------------
          int counter = 1;
          for(int i=0;i<BetaOptimization::betaSize;i++){

              if(BetaOptimization::isUsed[i])
              {
                  grad[counter] = -Smat_z(i);
                  counter++;
              }

          }

       }

       return s_z;
   case 5:

       // gradient for Givens Rotation constraint
       if(grad != NULL)
       {
           grad[0] = 0;
           grad[0] += ( pow(cosp,2) - pow(sinp,2) )*(s_x2-s_y2);
           grad[0] += (-1)*4*sinp*cosp*s_xy;
           //----------------------------------------
           int counter = 1;
           for(int i=0;i<BetaOptimization::betaSize;i++){

               if(BetaOptimization::isUsed[i])
               {
                   grad[counter] = 0;
                   grad[counter] += cosp*sinp*(Smat_y2(i)-Smat_x2(i));
                   grad[counter] += (pow(sinp,2)-pow(cosp,2))*Smat_xy(i);
                   counter++;
               }

           }
       }

       return cosp*sinp*(s_x2-s_y2)+(pow(cosp,2)-pow(sinp,2))*s_xy;
   }
   return -1;
}



/**
 * @brief spinItEnergyFunction Energy function for spin it optimization!
 * @param n dimension
 * @param x current value
 * @param grad gradient
 * @param my_func_data additional paramter (not used)
 * @return new value
 */
double BetaOptimization::spinItEnergyFunction(unsigned n, const double *x, double *grad, void *my_func_data)
{

    (void)n;

    GLint optimization_type = ((GLint*)my_func_data)[0];

    VectorXd betas(BetaOptimization::betaSize);
    GLdouble phi = x[0];

    int counter = 1;
    for(int i=0;i<BetaOptimization::betaSize;i++)
    {
        if(BetaOptimization::isUsed[i])
        {
            betas(i) = x[counter];
            counter++;
        }
        else
        {
            betas(i) = BetaOptimization::currentBetas[i];
        }
    }

    double cosp = cos(phi);
    double sinp = sin(phi);

    // current model volumes
    MatrixXd S = BetaOptimization::S_comp - BetaOptimization::S_mat*betas;

    // inertia tensor
    MatrixXd I(2,2);
    I(0,0) = s_y2+s_z2; I(0,1) = -s_xy;
    I(1,0) = -s_xy;     I(1,1) = s_x2+s_z2;

    if(IS_TOP_OPTIMIZATION)
    {
        MatrixXd diag11 = MatrixXd::Zero(2,2);
        diag11(0,0) = 1;
        diag11(1,1) = 1;

        // shifted inertia tensor
        I = I- (pow(s_z,2)/s_1) * diag11;
    }

    // rotation matrix
    MatrixXd R(2,2);
    R(0,0) = +cosp;R(0,1) = -sinp;
    R(1,0) = +sinp;R(1,1) = +cosp;

    // Givens Rotation representation
    MatrixXd mat = R*I*R.transpose();

    double IX = mat(0,0);
    double IY = mat(1,1);
    double IZ = s_x2+s_y2;


    double gc = gamma_c_top;
    double gi = IS_TOP_OPTIMIZATION?gamma_i_top:(IS_TIPPE_TOP_OPTIMIZATION?gamma_i_tippe_top:gamma_i_yoyo);
    double gl = IS_TOP_OPTIMIZATION?gamma_l_top:(IS_TIPPE_TOP_OPTIMIZATION?gamma_l_tippe_top:gamma_l_yoyo);

    double fFirstPart = 0;

    if(IS_TOP_OPTIMIZATION)
    {
        if(ONLY_LOWERING)
        {
            fFirstPart = gc*pow(s_z/s_1,2);
        }
        else
        {
            fFirstPart = gc*pow(s_z,2);
        }
    }

    double fSecondPart = 0;

    if(IS_TIPPE_TOP_OPTIMIZATION)
    {
        fSecondPart += pow(IX/IZ,2)+pow(IY/IZ,2);
        fSecondPart += pow(IX/IY,2)+pow(IZ/IY,2);
        fSecondPart += pow(IY/IX,2)+pow(IZ/IX,2);
        fSecondPart *= gi;
    }
    else
    {
        fSecondPart += gi*(pow(IX/IZ,2)+pow(IY/IZ,2));
    }

    double fThirdPart = gl*(0.5f)*(betas.transpose()*(L*betas))(0,0);

    double f = fFirstPart+fSecondPart+fThirdPart;

    //----------------------------------------

    // gradient for top energy function constraint
    if(grad != NULL)
    {

        // gradient depending on phi
        double dIxdphi = 2*cosp*sinp*(pow(s_x,2)-pow(s_y,2))+(pow(cosp,2)-pow(sinp,2))*s_xy;
        double dIydphi = 2*cosp*sinp*(pow(s_y,2)-pow(s_x,2))+(pow(sinp,2)-pow(cosp,2))*s_xy;

        if(IS_TIPPE_TOP_OPTIMIZATION)
        {
            grad[0] = 0;
            grad[0] += (dIxdphi*IX+dIydphi*IY)/pow(IZ,2);
            grad[0] += (dIxdphi*IX*IY-dIydphi*(pow(IX,2)+pow(IZ,2)))/pow(IZ,3);
            grad[0] += (dIydphi*IY*IX-dIxdphi*(pow(IY,2)+pow(IZ,2)))/pow(IZ,2);
            grad[0] *= 2*gi;
        }
        else
        {
            grad[0] = gi*(2/pow(IZ,2))*(dIxdphi*IX+dIydphi*IY);
        }       

        //----------------------------------------
        int counter = 1;
        for(int i=0;i<BetaOptimization::betaSize;i++){

            if(!BetaOptimization::isUsed[i])
            {
                continue;
            }

            SpMat vec(betas.size(),1);
            std::vector<T> coefficients;
            coefficients.push_back(T(i,0,1));
            vec.setFromTriplets(coefficients.begin(), coefficients.end());

            // gradient depending on beta i
            double d_sz_2_s1 = (pow(s_z,2)*Smat_1(i)-2*s_z*Smat_z(i)*s_1)/pow(s_1,2);
            double dIxb = 0;
            dIxb += (-1)*pow(cosp,2)*(Smat_y2(i)+Smat_z2(i)+d_sz_2_s1);
            dIxb += (-1)*pow(sinp,2)*(Smat_x2(i)+Smat_z2(i)+d_sz_2_s1);
            dIxb += (-1)*cosp*sinp*Smat_xy(i);
            double dIyb = 0;
            dIyb += (-1)*pow(sinp,2)*(Smat_y2(i)+Smat_z2(i)+d_sz_2_s1);
            dIyb += (-1)*pow(cosp,2)*(Smat_x2(i)+Smat_z2(i)+d_sz_2_s1);
            dIyb += cosp*sinp*Smat_xy(i);
            double dIzb = -(Smat_x2(i)+Smat_y2(i));

            double dFirstPart = 0;

            if(IS_TOP_OPTIMIZATION)
            {
                if(ONLY_LOWERING)
                {
                    dFirstPart = gc*(2)*(s_z/pow(s_1,3))*(s_z*Smat_1(i)-Smat_z(i)*s_1);
                }
                else
                {
                    dFirstPart = gc*(-2)*s_z*Smat_z(i);
                }
            }

            double dSecondPart = gi*(2/pow(IZ,3))*( dIxb*IZ*IX + dIyb*IZ*IY - dIzb*(pow(IX,2)+pow(IY,2)));
            double dThirdPart = gl*(betas.transpose()*(L*vec))(0,0);


            if(IS_TIPPE_TOP_OPTIMIZATION)
            {
                dSecondPart = 0;
                dSecondPart += (dIxb*IX*IZ + dIyb*IY*IZ - dIzb*(pow(IX,2)+pow(IY,2)))/pow(IZ,3);
                dSecondPart += (dIxb*IX*IY + dIzb*IZ*IY - dIyb*(pow(IX,2)+pow(IZ,2)))/pow(IY,3);
                dSecondPart += (dIyb*IY*IX + dIzb*IZ*IX - dIxb*(pow(IY,2)+pow(IZ,2)))/pow(IX,3);
                dSecondPart *= gi*2;
            }
            else
            {
                dSecondPart = gi*(2/pow(IZ,3))*( dIxb*IZ*IX + dIyb*IZ*IY - dIzb*(pow(IX,2)+pow(IY,2)));
            }

            grad[counter] = dFirstPart+dSecondPart+dThirdPart;
            counter++;
        }
    }
    //----------------------------------------

    return f;
}

/**
 * @brief BetaOptimization::executeBetasOptimization Execute beta optimization.
 * @param cubeVector
 * @param optimizationType
 * @return
 */
bool BetaOptimization::executeBetasOptimization(QVector<octree::cubeObject>* cubeVector,GLint optimizationType)
{

    double minf;
    int counter;

    int usedCubeCounter = 0;

    BetaOptimization::isUsed = new bool[cubeVector->size()];
    BetaOptimization::currentBetas = new double[cubeVector->size()];
    BetaOptimization::betaSize = cubeVector->size();

    for(int i=0;i<cubeVector->size();i++)
    {
        octree::cubeObject* obj = &cubeVector->data()[i];
        bool isUsed = !IGNORE_BINARIES || ((FIXED_EPSILON)<obj->beta && obj->beta<(1-FIXED_EPSILON));
        BetaOptimization::isUsed[i] = isUsed;
        BetaOptimization::currentBetas[i] = obj->beta;
        usedCubeCounter += isUsed ?1:0;
    }

    cout << "----------------------------------------" << endl;
    cout << "number of cubes (comp): " << cubeVector->size() << endl;
    cout << "number of cubes (used): " << usedCubeCounter << endl;

    if(usedCubeCounter>MAX_NUMBER_OF_VARIABLES)
    {
        cout << "----------------------------------------" << endl;
        cout << "abort execution of optimization" << endl;
        cout << "because max number of variables is reached!" << endl;
        return false;
    }

    //------------------------------
    int bs = usedCubeCounter+1;
    //------------------------------


    // lower and upper borders (0,1) for betas
    double* lb_si = new double[bs];
    double* ub_si = new double[bs];
    double* betas = new double[bs];

    counter = 1;
    for(int i=0;i<cubeVector->size();i++)
    {
        if(BetaOptimization::isUsed[i])
        {
            lb_si[counter] = 0.0;
            ub_si[counter] = 1.0;
            betas[counter] = cubeVector->data()[i].beta;
            counter++;
        }
    }

    // lower and upper borders (-pi,+pi) for phi
    lb_si[0] = -PI;
    ub_si[0] = +PI;
    betas[0] = BetaOptimization::phi;

    //------------------------------
    nlopt_opt opt_spin_it;
    opt_spin_it = nlopt_create( USED_ALGO, bs);

    // set borders
    nlopt_set_lower_bounds(opt_spin_it, lb_si);
    nlopt_set_upper_bounds(opt_spin_it, ub_si);

    GLint optimization_type = optimizationType;

    // set optimization function
    nlopt_set_min_objective(opt_spin_it, spinItEnergyFunction, &optimization_type);

    double val1 = OPTIMIZATION_FUNCTION_THRESHOLD;
    double val2 = OPTIMIZATION_CONSTAINTS_THRESHOLD;

    // set tolerance value
    nlopt_set_xtol_rel(opt_spin_it, val1);


    spin_it_constraint_data data_si[10] = { {0},{1},{2},{3},{4},{5} };
    nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[0], val2);
    nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[1], val2);
    nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[2], val2);
    nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[3], val2);
    if(optimizationType==OPTIMIZATION_TYPE_YOYO)
    {
        nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[4], val2);
    }
    nlopt_add_equality_constraint(opt_spin_it, spinItContraints, &data_si[5], val2);


    // set maximal time
    nlopt_set_maxtime(opt_spin_it,MAX_TIME);

    // execute optimization
    nlopt_result result =  nlopt_optimize(opt_spin_it, betas, &minf);

    cout << "----------------------------------------" << endl;
    if ( result < 0)
    {

        cout << "optimization failed!" << endl;

        if(result == NLOPT_FAILURE)
        {
            cout << "Generic failure code!" << endl;
        }
        else
        if(result == NLOPT_INVALID_ARGS)
        {
            cout << "Invalid arguments (e.g. lower bounds are bigger than upper bounds, an unknown algorithm was specified, etcetera)!" << endl;
        }
        else
        if(result == NLOPT_OUT_OF_MEMORY)
        {
            cout << "Ran out of memory!" << endl;
        }
        else
        if(result == NLOPT_ROUNDOFF_LIMITED)
        {
            cout << "Halted because roundoff errors limited progress! (In this case, the optimization still typically returns a useful result.)" << endl;
        }
        else
        if(result == NLOPT_FORCED_STOP)
        {
            cout << "Forced stop!" << endl;
        }

        nlopt_destroy(opt_spin_it);
        delete lb_si;
        delete ub_si;
        delete betas;
        delete BetaOptimization::isUsed;
        delete BetaOptimization::currentBetas;
        BetaOptimization::isUsed = NULL;
        BetaOptimization::currentBetas = NULL;
        BetaOptimization::betaSize = -1;

        return false;

    }
    else
    {
        cout << "optimization successful (found minimum " << minf << ")" << endl;

        if(result == NLOPT_SUCCESS){
            cout << "Generic success return value!" << endl;
        }
        else
        if(result == NLOPT_STOPVAL_REACHED){
            cout << "Optimization stopped because stopval (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_FTOL_REACHED){
            cout << "Optimization stopped because ftol_rel or ftol_abs (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_XTOL_REACHED){
            cout << "Optimization stopped because xtol_rel or xtol_abs (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_MAXEVAL_REACHED){
            cout << "Optimization stopped because maxeval (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_MAXTIME_REACHED){
            cout << "Optimization stopped because maxtime (above) was reached!" << endl;
        }

    }

    // update betas
    counter = 1;
    for(int i=0;i<cubeVector->size();i++)
    {
        if(BetaOptimization::isUsed[i])
        {
            octree::cubeObject* obj = &cubeVector->data()[i];
            obj->beta = betas[counter];
            counter++;
        }
    }

    BetaOptimization::phi = betas[0];

    nlopt_destroy(opt_spin_it);
    delete lb_si;
    delete ub_si;
    delete betas;
    delete BetaOptimization::isUsed;
    delete BetaOptimization::currentBetas;
    BetaOptimization::isUsed = NULL;
    BetaOptimization::currentBetas = NULL;
    BetaOptimization::betaSize = -1;

    return true;
}

/**
 * @brief BetaOptimization::resetPhi Reset phi.
 */
void BetaOptimization::resetPhi()
{
    BetaOptimization::phi = 0.f;
}

/**
 * @brief BetaOptimization::calcInertiaTensor Output moment of inertia properties
 * @param S_comp
 * @param S_mat
 * @param cubeVector
 * @param phi
 * @param useMinMaxValues
 */
void BetaOptimization::showProperties(
        VectorXd S_comp,
        MatrixXd S_mat,
        QVector<octree::cubeObject>* cubeVector,
        double phi,
        bool useMinMaxValues)
{

    cout << "----------" << "----------" << endl;
    if(useMinMaxValues)
    {
        cout << "Inertia Properties (with Min and Max):" << endl;
    }
    else
    {
        cout << "Inertia Properties:" << endl;
    }


    double cosp = cos(phi);
    double sinp = sin(phi);

    VectorXd betas;

    if(cubeVector != NULL){
        betas.resize(cubeVector->size());
        for (int i = 0; i < cubeVector->size(); i++) {

            if(useMinMaxValues)
            {
                betas(i) = cubeVector->data()[i].beta>0.5f?1.f:0.f;
            }
            else
            {
                betas(i) = cubeVector->data()[i].beta;
            }

        }
    }

    // current model volumes
    MatrixXd S = cubeVector != NULL ? S_comp - S_mat*betas : S_comp;

    // inertia tensor
    MatrixXd I(3,3);
    I(0,0) = s_y2+s_z2; I(0,1) = -s_xy;     I(0,2) = -s_xz;
    I(1,0) = -s_xy;     I(1,1) = s_x2+s_z2; I(1,2) = -s_yz;
    I(2,0) = -s_xz;     I(2,1) = -s_yz;     I(2,2) = s_x2+s_y2;

    /*
    cout << "----------" << endl;
    cout << "Inertia Matrix:" << endl;
    cout << I << endl;
    */

    // rotation matrix
    MatrixXd R(2,2);
    R(0,0) = +cosp;R(0,1) = -sinp;
    R(1,0) = +sinp;R(1,1) = +cosp;

    MatrixXd diag110 = MatrixXd::Zero(3,3);
    diag110(0,0) = 1;
    diag110(1,1) = 1;

    // shifted inertia tensor
    MatrixXd I_com = I - (pow(s_z,2)/s_1) * diag110;

    cout << "----------" << endl;
    cout << "Shifted Inertia Matrix:" << endl;
    cout << I_com << endl;

    MatrixXd I_2(2,2);
    I_2(0,0) = I_com(0,0);    I_2(0,1) = I_com(0,1);
    I_2(1,0) = I_com(1,0);    I_2(1,1) = I_com(1,1);

    // Givens Rotation representation
    MatrixXd mat = R*I_2*R.transpose();

    cout << "----------------------------------------" << endl;
    cout << "Rotated Component:" << endl;
    cout << mat << endl;

    // center of mass
    VectorXd c(3);
    c(0) = s_x;
    c(1) = s_y;
    c(2) = s_z;
    c =c/s_1;

    cout << "----------------------------------------" << endl;
    cout << "Center of Mass:" << endl;
    cout << c << endl;

    cout << "----------------------------------------" << endl;
    cout << "Mass:" << endl;
    cout << s_1 << endl;

    cout << "----------------------------------------" << endl;
    cout << "phi:" << endl;
    cout << phi << endl;

    /*
    if(cubeVector != NULL && cubeVector->size()<=10)
    {

        cout << "----------------------------------------" << endl;
        cout << "L:" << endl;
        cout << L.toDense() << endl;

    }
    */

}

/**
 * @brief BetaOptimization::setSForCompleteMesh Set the integral vector for the entire mesh.
 */
void  BetaOptimization::setSForCompleteMesh()
{
    BetaOptimization::S_comp.resize(10);

    float* s = calculateVolume(BetaOptimization::mesh);
    for (int i = 0; i < 10; i++)
    {
        BetaOptimization::S_comp(i) = s[i];
    }

}

/**
 * @brief BetaOptimization::setSMatrixForCubes Set the integral matrix for the currently considered cubes.
 * @param cubeVector
 */
void  BetaOptimization::setSMatrixForCubes(QVector<octree::cubeObject>* cubeVector)
{

    BetaOptimization::S_mat.resize(10,cubeVector->size());

    for (int i = 0; i < cubeVector->size(); i++)
    {
        float* s = calculateVolume(cubeVector->at(i).mesh);
        for (int j = 0; j < 10; j++)
        {
            BetaOptimization::S_mat(j,i) = s[j];
        }
    }

}

/**
 * @brief setCheckMatrixForCubes Get matrix for testing!
 */
void BetaOptimization::setCheckMatrixForCubes()
{
    QVector<octree::cubeObject>* cubeVector = BetaOptimization::octree.getInnerCubes();

    BetaOptimization::S_inner_comp.resize(10);
    BetaOptimization::S_inner_comp.setZero();

    for (int i = 0; i < cubeVector->size(); i++)
    {
        float* s = calculateVolume(cubeVector->at(i).mesh);
        for (int j = 0; j < 10; j++)
        {
            BetaOptimization::S_inner_comp(j) += s[j];
        }
    }

    cout << "----------------------------------------" << endl;
    cout << "complete inner volume" << endl;
    cout << BetaOptimization::S_inner_comp << endl;

}

/**
 * @brief BetaOptimization::getEpsilon Choosing a specific elpsilon value for a depth.
 * @param depth depth used for finding an appropriate epsilon (currently not used)
 * @return epsilon value
 */
GLfloat BetaOptimization::getEpsilon(GLint depth)
{

    (void)depth;

    return FIXED_EPSILON;

}

/**
 * @brief BetaOptimization::optimizeBetasBottomUp Optimize betas (bottum up).
 * @param optimizationType
 */
void BetaOptimization::optimizeBetasBottomUp(GLint optimizationType)
{

    cout << "----------------------------------------" << endl;
    cout << "Beta Optimization Bottom Up" << endl;

    GLint depth = BetaOptimization::octree.getOptimizationMaxDepth();

    for (int i = BOTTOM_UP_START_DEPTH; i < depth; i++) {

        QVector<octree::cubeObject>* cubeVector = BetaOptimization::octree.getCubesOfLowerDepth(i);
        BetaOptimization::setSMatrixForCubes(cubeVector);
        BetaOptimization::L = BetaOptimization::octree.getUniformLaplace(cubeVector);

        showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

        bool wasSuccessful = BetaOptimization::executeBetasOptimization(cubeVector,optimizationType);

        if(!wasSuccessful)
        {
            cout << "----------------------------------------" << endl;
            cout << "abort optimization (bottom up)" << endl;
            break;
        }

        showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

        BetaOptimization::octree.updateBetaValuesWithPropagation();

        GLfloat epsilon = BetaOptimization::getEpsilon(BetaOptimization::octree.getOptimizationMaxDepth());
        cout << "----------------------------------------" << endl;
        if(epsilon<0.f)
        {
            cout << "no fittest epsilon has been founded!" << endl;
            break;
        }
        else
        {
            cout << "fittest epsilon is " << epsilon << "!" << endl;
        }

        BetaOptimization::octree.splitAndMerge(epsilon,BetaOptimization::octree.getOptimizationMaxDepth());

    }

    BetaOptimization::finishBetaOptimization();

}

/**
 * @brief BetaOptimization::optimizeBetas Optimize betas (simple).
 * @param optimizationType
 */
void BetaOptimization::optimizeBetas(int optimizationType)
{

    cout << "----------------------------------------" << endl;
    cout << "Beta Optimization" << endl;

    QVector<octree::cubeObject>* cubeVector = BetaOptimization::octree.getMergedCubes();

    cout << "----------------------------------------" << endl;
    cout << "number of cubes is " << cubeVector->size() << endl;

    BetaOptimization::setSMatrixForCubes(cubeVector);
    BetaOptimization::L = BetaOptimization::octree.getUniformLaplace(cubeVector);

    BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

    BetaOptimization::executeBetasOptimization(cubeVector,optimizationType);

    BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

    BetaOptimization::octree.updateBetaValues();

    BetaOptimization::finishBetaOptimization();

}

/**
 * @brief BetaOptimization::optimizeBetasWithSplitAndMerge Optimize betas (with split and merge).
 * @param optimizationType
 */
void BetaOptimization::optimizeBetasWithSplitAndMerge(int optimizationType)
{

    bool notConverged =  true;

    cout << "----------------------------------------" << endl;
    cout << "Beta Optimization with Split and Merge Step" << endl;

    GLint counter=1;

    while(notConverged)
    {

        cout << "----------------------------------------" << endl;
        cout << "Split and Merge Step (" << counter << ")" << endl;
        counter++;

        QVector<octree::cubeObject>* cubeVector = BetaOptimization::octree.getMergedCubes();
        BetaOptimization::setSMatrixForCubes(cubeVector);
        BetaOptimization::L = BetaOptimization::octree.getUniformLaplace(cubeVector);

        BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

        bool wasSuccessful = BetaOptimization::executeBetasOptimization(cubeVector,optimizationType);

        if(!wasSuccessful)
        {
            cout << "----------------------------------------" << endl;
            cout << "abort optimization (split and merge)" << endl;
            break;
        }

        BetaOptimization::showProperties(S_comp,S_mat,cubeVector,BetaOptimization::phi);

        BetaOptimization::octree.updateBetaValuesWithPropagation();

        GLfloat epsilon = BetaOptimization::getEpsilon(BetaOptimization::octree.getOptimizationMaxDepth());
        cout << "----------------------------------------" << endl;
        if(epsilon<0.f)
        {
            cout << "no fittest epsilon has been founded!" << endl;
            break;
        }
        else
        {
            cout << "fittest epsilon is " << epsilon << "!" << endl;
        }

        notConverged = BetaOptimization::octree.splitAndMerge(epsilon,BetaOptimization::octree.getOptimizationMaxDepth());

    }

    BetaOptimization::octree.deleteNodeMeshes();

}

/**
 * @brief BetaOptimization::testSimpleSplitAndMerge Test function!
 */
void BetaOptimization::testSimpleSplitAndMerge()
{

    QVector<octree::cubeObject>* cubeVector = NULL;

    cubeVector = BetaOptimization::octree.getCubesOfLowerDepth(3);

    for (int i = 0; i < cubeVector->size(); i++) {
        octree::cubeObject* obj = &cubeVector->data()[i];

        QVector<GLfloat>* vec = obj->mesh->getGeometry();

        bool above = false;
        bool under = false;

        for(int i=0;i<vec->size();i+=3)
        {
            //GLfloat x = vec->data()[i+0];
            GLfloat y = vec->data()[i+1];
            GLfloat z = vec->data()[i+2];

            if(z<3.5+y*2)
            {
                above = true;
            }
            else
            {
                under = true;
            }

        }

        obj->beta = above && !under ? 1.0 : (under && !above? 0.0 : 0.5);
    }

    BetaOptimization::octree.updateBetaValuesWithPropagation();

    BetaOptimization::octree.deleteNodeMeshes();

    BetaOptimization::finishBetaOptimization();

}

/**
 * @brief BetaOptimization::testSplitAndMerge Test function!
 */
void BetaOptimization::testSplitAndMerge()
{

    QVector<octree::cubeObject>* cubeVector = NULL;

    bool not_converged = true;
    while (not_converged)
    {

        // get the inner cubes of the octree
        cubeVector = BetaOptimization::octree.getMergedCubes();

        for (int i = 0; i < cubeVector->size(); i++) {
            octree::cubeObject* obj = &cubeVector->data()[i];

            QVector<GLfloat>* vec = obj->mesh->getGeometry();

            bool above = false;
            bool under = false;

            for(int i=0;i<vec->size();i+=3)
            {
                //GLfloat x = vec->data()[i+0];
                GLfloat y = vec->data()[i+1];
                GLfloat z = vec->data()[i+2];

                if(z<3.5+y*2)
                {
                    above = true;
                }
                else
                {
                    under = true;
                }

            }

            obj->beta = above && !under ? 1.0 : (under && !above? 0.0 : 0.5);
        }

        // set new betas and clrea cubeVector
        BetaOptimization::octree.updateBetaValuesWithPropagation();

        // do split and merge
        not_converged = BetaOptimization::octree.splitAndMerge(0.001,1000);
    }

    BetaOptimization::octree.deleteNodeMeshes();

    BetaOptimization::finishBetaOptimization();

}

/**
 * @brief BetaOptimization::getShellMesh Get shell mesh which must be translated.
 * @param flipped flip normals
 * @return shell mesh
 */
Mesh* BetaOptimization::getShellMesh(bool flipped)
{
    Mesh* mesh = BetaOptimization::octree.getShellMesh(flipped);

    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.translate(BetaOptimization::com2);
    mesh->transform(mat);

    return mesh;
}

/**
 * @brief BetaOptimization::finishBetaOptimization Execute task after optimization.
 */
void BetaOptimization::finishBetaOptimization()
{


    // set each cube of the octree either to void (beta>0.5) or not void (beta<=0.5)
    BetaOptimization::octree.setVoids();

    BetaOptimization::octree.createInnerSurface();
    Mesh* newShellMesh = BetaOptimization::getShellMesh();

    if(shellMesh != NULL)
    {
       delete shellMesh;
    }

    shellMesh = newShellMesh;

    BetaOptimization::octree.makeDirty();

}

/**
 * @brief BetaOptimization::calculateVolume calculates volumne integrals of an object.
 * (s_1,s_x,s_y,s_z,s_xy,s_yz,s_xz,s_x2,s_y2,s_z2)
 * @param mesh triangulated surface of object
 * @param p density of object (default 1)
 * @return pointer of array
 */
float* BetaOptimization::calculateVolume(Mesh* mesh, float p)
{
    for (int i = 0; i < 10; i++) {
        BetaOptimization::mesh_volume[i] = 0;
    }
    QVector<float>* geometry = mesh->getGeometry();
    QVector<float>* normals = mesh->getSurfaceNormals();
    QVector<int>* indices = mesh->getIndices();

    for (int i = 0; i < indices->size(); i += 3) {
        QVector3D a;
        a.setX(geometry->at(3 * indices->at(i)));
        a.setY(geometry->at(3 * indices->at(i) + 1));
        a.setZ(geometry->at(3 * indices->at(i) + 2));

        QVector3D b;
        b.setX(geometry->at(3 * indices->at(i + 1)));
        b.setY(geometry->at(3 * indices->at(i + 1) + 1));
        b.setZ(geometry->at(3 * indices->at(i + 1) + 2));

        QVector3D c;
        c.setX(geometry->at(3 * indices->at(i + 2)));
        c.setY(geometry->at(3 * indices->at(i + 2) + 1));
        c.setZ(geometry->at(3 * indices->at(i + 2) + 2));

        QVector3D n;
        n.setX(normals->at(i));
        n.setY(normals->at(i + 1));
        n.setZ(normals->at(i + 2));

        QVector3D h1 = a + b + c;
        QVector3D h2 = a*a + b*(a + b);
        QVector3D h3 = h2 + c*h1;
        QVector3D h4 = a*a*a + b*h2 + c*h3;
        QVector3D h5 = h3 + a*(h1 + a);
        QVector3D h6 = h3 + b*(h1 + b);
        QVector3D h7 = h3 + c*(h1 + c);
        QVector3D h8 = QVector3D(a.y(),a.z(),a.x())*h5 + QVector3D(b.y(),b.z(),b.x())*h6 + QVector3D(c.y(),c.z(),c.x())*h7;

        QVector3D si;
        BetaOptimization::mesh_volume[0] += (n*h1).x();
        si = n*h3;
        BetaOptimization::mesh_volume[1] += si.x();
        BetaOptimization::mesh_volume[2] += si.y();
        BetaOptimization::mesh_volume[3] += si.z();
        si = n*h8;
        BetaOptimization::mesh_volume[4] += si.x();
        BetaOptimization::mesh_volume[5] += si.y();
        BetaOptimization::mesh_volume[6] += si.z();
        si = n*h4;
        BetaOptimization::mesh_volume[7] += si.x();
        BetaOptimization::mesh_volume[8] += si.y();
        BetaOptimization::mesh_volume[9] += si.z();
    }

    BetaOptimization::mesh_volume[0] *= (float) 1/6;
    for (int i = 1; i <= 3; i++) {
        BetaOptimization::mesh_volume[i] *= (float) 1/24;
    }
    for (int i = 4; i <= 6; i++) {
        BetaOptimization::mesh_volume[i] *= (float) 1/120;
    }
    for (int i = 7; i <= 9; i++) {
        BetaOptimization::mesh_volume[i] *= (float) 1/60;
    }

    for (int i = 0; i < 10; i++) {
        BetaOptimization::mesh_volume[i] *= p;
    }

    return BetaOptimization::mesh_volume;
}

/**
 * @brief BetaOptimization::phiEnergyFunction Energy function for phi optimization.
 * @param n
 * @param x
 * @param grad
 * @param my_func_data
 * @return
 */
double BetaOptimization::phiEnergyFunction(unsigned n, const double *x, double *grad, void *my_func_data)
{
    (void)n;
    (void)my_func_data;

    GLdouble phi = x[0];

    double cosp = cos(phi);
    double sinp = sin(phi);

    // current model volumes
    MatrixXd S = BetaOptimization::S_comp;

    double f = cosp*sinp*(s_x2-s_y2)+(pow(cosp,2)-pow(sinp,2))*s_xy;

    // gradient for Givens Rotation constraint
    if(grad != NULL)
    {
        double firstPart = f;
        double secondPart = 0;

        secondPart += ( pow(cosp,2) - pow(sinp,2) )*(s_x2-s_y2);
        secondPart += (-1)*4*sinp*cosp*s_xy;

        grad[0] = 2*firstPart*secondPart;

    }

    return pow(f,2);

}

/**
 * @brief BetaOptimization::executePhiOptimization Execute phi optimization.
 * @return
 */
double BetaOptimization::executePhiOptimization()
{

    double minf;

    double lb_si = -PI;
    double ub_si = +PI;
    double phi = 0;

    //------------------------------
    nlopt_opt opt_phi;
    opt_phi = nlopt_create( USED_ALGO, 1);

    // set borders
    nlopt_set_lower_bounds(opt_phi, &lb_si);
    nlopt_set_upper_bounds(opt_phi, &ub_si);

    // set optimization function
    nlopt_set_min_objective(opt_phi, phiEnergyFunction, NULL);

    double val1 = OPTIMIZATION_FUNCTION_THRESHOLD;

    // set tolerance value
    nlopt_set_xtol_rel(opt_phi, val1);

    // set maximal time
    nlopt_set_maxtime(opt_phi,MAX_TIME);

    // execute optimization
    nlopt_result result =  nlopt_optimize(opt_phi, &phi, &minf);

    cout << "----------------------------------------" << endl;
    if ( result < 0)
    {

        cout << "optimization failed!" << endl;

        if(result == NLOPT_FAILURE)
        {
            cout << "Generic failure code!" << endl;
        }
        else
        if(result == NLOPT_INVALID_ARGS)
        {
            cout << "Invalid arguments (e.g. lower bounds are bigger than upper bounds, an unknown algorithm was specified, etcetera)!" << endl;
        }
        else
        if(result == NLOPT_OUT_OF_MEMORY)
        {
            cout << "Ran out of memory!" << endl;
        }
        else
        if(result == NLOPT_ROUNDOFF_LIMITED)
        {
            cout << "Halted because roundoff errors limited progress! (In this case, the optimization still typically returns a useful result.)" << endl;
        }
        else
        if(result == NLOPT_FORCED_STOP)
        {
            cout << "Forced stop!" << endl;
        }

    }
    else
    {
        cout << "optimization successful (found minimum " << minf << ")" << endl;

        if(result == NLOPT_SUCCESS){
            cout << "Generic success return value!" << endl;
        }
        else
        if(result == NLOPT_STOPVAL_REACHED){
            cout << "Optimization stopped because stopval (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_FTOL_REACHED){
            cout << "Optimization stopped because ftol_rel or ftol_abs (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_XTOL_REACHED){
            cout << "Optimization stopped because xtol_rel or xtol_abs (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_MAXEVAL_REACHED){
            cout << "Optimization stopped because maxeval (above) was reached!" << endl;
        }
        else
        if(result == NLOPT_MAXTIME_REACHED){
            cout << "Optimization stopped because maxtime (above) was reached!" << endl;
        }

    }

    nlopt_destroy(opt_phi);

    return phi;
}
