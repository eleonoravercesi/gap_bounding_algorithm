#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include <vector>
#include <iostream>

using namespace std;

double opt_plus(vector<double> x, int ncount) {
    SCIP* scip = nullptr;
    SCIP_CALL(SCIPcreate(&scip));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    SCIP_CALL(SCIPcreateProbBasic(scip, "Gap plus model"));

    // 1. Create variables (Upper Triangle Only)
    int cont = 0;
    vector<vector<SCIP_VAR*>> c(ncount, vector<SCIP_VAR*>(ncount, nullptr));

    for (int i = 0; i < ncount; ++i) {
        for (int j = i + 1; j < ncount; ++j) {
            char vname[64];
            snprintf(vname, sizeof(vname), "c_%d_%d", i, j);

            // Check bounds for x
            double obj_coef = (cont < x.size()) ? x[cont] : 0.0;

            SCIP_CALL(SCIPcreateVarBasic(scip, &c[i][j], vname,
                                         0.0, SCIPinfinity(scip), obj_coef, SCIP_VARTYPE_CONTINUOUS));
            SCIP_CALL(SCIPaddVar(scip, c[i][j]));

            // Mirror the pointer so c[j][i] points to the same SCIP_VAR
            c[j][i] = c[i][j];
            cont++;
        }
    }

    // 2. Constraints: triangle inequalities
    for (int i = 0; i < ncount; ++i) {
        for (int j = i + 1; j < ncount; ++j) {
            for (int k = j + 1; k < ncount; ++k) {
                // Triplets (i,j,k) -> we need 3 inequalities
                SCIP_VAR* vars[3][3] = {
                    {c[i][j], c[i][k], c[k][j]}, // c[i][j] <= c[i][k] + c[k][j]
                    {c[i][k], c[i][j], c[j][k]}, // c[i][k] <= c[i][j] + c[j][k]
                    {c[j][k], c[i][j], c[i][k]}  // c[j][k] <= c[i][j] + c[i][k]
                };

                for (int m = 0; m < 3; ++m) {
                    SCIP_CONS* cons = nullptr;
                    char cname[64];
                    snprintf(cname, sizeof(cname), "tri_%d_%d_%d_%d", i, j, k, m);

                    // LHS: -inf, RHS: 0.0  =>  vars[0] - vars[1] - vars[2] <= 0
                    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, cname, 0, nullptr, nullptr,
                                                        -SCIPinfinity(scip), 0.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, vars[m][0], 1.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, vars[m][1], -1.0));
                    SCIP_CALL(SCIPaddCoefLinear(scip, cons, vars[m][2], -1.0));
                    SCIP_CALL(SCIPaddCons(scip, cons));
                    SCIP_CALL(SCIPreleaseCons(scip, &cons)); // Release immediately after adding
                }
            }
        }
    }

    SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));
    SCIP_CALL(SCIPsolve(scip));

    SCIP_SOL* sol = SCIPgetBestSol(scip);
    double opt = (sol != nullptr) ? SCIPgetSolOrigObj(scip, sol) : 0.0;

    // Cleanup Variables
    for (int i = 0; i < ncount; ++i) {
        for (int j = i + 1; j < ncount; ++j) {
            SCIP_CALL(SCIPreleaseVar(scip, &c[i][j]));
        }
    }

    SCIP_CALL(SCIPfree(&scip));
    return opt;
}