#ifndef MEASUREHANDLER_H
#define MEASUREHANDLER_H

#include "handlers/handler.h"

#include <Python.h>


/**
 * @class MeasureHandler
 * @brief This class is designed to call a Python functions each time `make()` is called.
 *
 * The `inputFile`, MUST follow the following pattern:
 *
 * \code{.py}
 * import numpy as np
 *
 * def init(BPMx_nb, BPMy_nb, CMx_nb, CMy_nb):
 *      global gBPMx_nb, gBPMy_nb, gCMx_nb, gCMy_nb
 *      global ...other global....
 *
 *      gBPMx_nb = BPMx_nb
 *      gBPMy_nb = BPMy_nb
 *      gCMx_nb = CMx_nb
 *      gCMy_nb = CMy_nb
 *
 *      gXXX = .... initialization of other variables ....
 *
 *
 * def corr_value(BPMx, BPMy):
 *     global gBPMx_nb, gBPMy_nb, gCMx_nb, gCMy_nb
 *     CMx = np.array(gCMx_nb)
 *     CMy = nb.array(gCMy_nb)
 *
 *     ...do something here...
 *
 *     return CMx, CMy
 * \endcode
 */
class MeasureHandler : public Handler
{
public:
    explicit MeasureHandler(RFMDriver *driver, DMA *dma, bool weightedCorr,
                            std::string inputFile);
    ~MeasureHandler();
    virtual int make();

private:
    /**
     * @brief Set the processor, here Python.
     */
    void setProcessor(arma::mat SmatX, arma::mat SmatY,
                      double IvecX, double IvecY,
                      double Frequency,
                      double P, double I, double D,
                      arma::vec CMx, arma::vec CMy, bool weightedCorr);
    void setModule();

    /**
     * @brief Itinitialize Python environnment.
     *
     * @note Should be called only after having set `m_numBPMx/y` and `m_numCMx/y`!
     * @return Error: 0 if success, 1 if failure.
     */
    int initPython();

    /**
     */
    int callPythonInit();

    /**
     * @brief Calls the python function defined in the constructor.
     */
    int callPythonFunction(const arma::vec &BPMx, const arma::vec &BPMy,
                           arma::vec &CMx, arma::vec &CMy);

    PyObject *m_pFunc;
    PyObject *m_pModule;
    std::string m_inputFile;
    std::string m_inputPath;
    std::string m_inputModule;
    std::string m_functionName;
};

#endif // MEASUREHANDLER_H
