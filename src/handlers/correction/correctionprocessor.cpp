#include "correctionprocessor.h"

#include "adc.h"

CorrectionProcessor::CorrectionProcessor()
{
    m_lastrmsX = 999;
    m_lastrmsY = 999;
    m_rmsErrorCnt = 0;
}

void CorrectionProcessor::setCMs(arma::vec CMx, arma::vec CMy)
{
    m_CMx = CMx;
    m_CMy = CMy;

    m_dCORlastX  = arma::zeros<arma::vec>(m_CMx.n_elem); 
    m_dCORlastY  = arma::zeros<arma::vec>(m_CMy.n_elem);
    m_Xsum       = arma::zeros<arma::vec>(m_CMx.n_elem);
    m_Ysum       = arma::zeros<arma::vec>(m_CMy.n_elem);
}

void CorrectionProcessor::setSmat(arma::mat &SmatX, arma::mat &SmatY, double IvecX, double IvecY, bool weightedCorr)
{
    m_useCMWeight = weightedCorr;
    this->calcSmat(SmatX, IvecX, m_CMWeightX, m_SmatInvX);
    this->calcSmat(SmatY, IvecY, m_CMWeightY, m_SmatInvY);
}

void CorrectionProcessor::setInjectionCnt(double frequency)
{
    m_injectionCnt = 0;
    m_injectionStartCnt = (int) frequency/1000;
    m_injectionStopCnt  = (int) frequency*60/1000;
}

int CorrectionProcessor::correct(arma::vec &diffX, arma::vec &diffY,
                                 bool newInjection,
                                 arma::vec &Data_CMx, arma::vec &Data_CMy,
                                 int type)
{
    static int count_test=0;
    static int count=0;
    ++count;
    if (count%15000==0) {
        count_test+=20;
        count=0;
    }

    if (sum(diffX) < -10.5) {
        std::cout << " ERROR: No Beam" << std::endl;
      //  return FOFB_ERROR_NoBeam;
    }

    if ( newInjection )
    {
        m_injectionCnt += 1;
        if ((m_injectionCnt >= m_injectionStopCnt) && (m_injectionCnt <= m_injectionStartCnt))
            return 0;
    }
    m_injectionCnt = 0;

    //cout << "  prove rms" << endl;
    double rmsX = (diffX.n_elem-1) * stddev(diffX) / diffX.n_elem;
    double rmsY = (diffY.n_elem-1) * stddev(diffY) / diffY.n_elem;
    if ((rmsX > m_lastrmsX*1.1) || (rmsY > m_lastrmsY*1.1))
    {
        m_rmsErrorCnt++;
        if (m_rmsErrorCnt > 5) {
            return FOFB_ERROR_RMS;
        }
    } else {
        m_rmsErrorCnt = 0;
    }
    m_lastrmsX = rmsX;
    m_lastrmsY = rmsY;

    //cout << "  calc dCOR" << endl;
    arma::vec dCMx = m_SmatInvX * diffX;
    arma::vec dCMy = m_SmatInvY * diffY;

    if (m_useCMWeight) {
        dCMx = dCMx % m_CMWeightX;
        dCMy = dCMy % m_CMWeightY;
    }

//    if ((max(dCMx) > 0.100) || (max(dCMy) > 0.100))
  //      return FOFB_ERROR_CM100;


    //cout << "  calc PID" << endl;
    if (m_currentP < m_P)
        m_currentP += 0.01;

    if ((type & Correction::Horizontal) == Correction::Horizontal) {
        m_dCORxPID  = (dCMx * m_currentP) + (m_I*m_Xsum)  + (m_D*(dCMx-m_dCORlastX));
        m_dCORlastX = dCMx;
        m_Xsum      = m_Xsum+dCMx;
        m_CMx       = m_CMx - m_dCORxPID;
        Data_CMx = m_CMx;
    }

    if ((type & Correction::Vertical) == Correction::Vertical) {
        m_dCORyPID  = (dCMy * m_currentP) + (m_I*m_Ysum)  + (m_D*(dCMy-m_dCORlastY));
        m_dCORlastY = dCMy;
        m_Ysum      = m_Ysum+dCMy;
        m_CMy       = m_CMy - m_dCORyPID;
        Data_CMy    = m_CMy;
    }

    return 0;
}

int CorrectionProcessor::checkCorrection()
{
 /* --- start timer --- */
 // t_dac_start.clock();    
/*
  RFM2GEVENTINFO EventInfo;           // Info about received interrupts  
  RFM2G_NODE     otherNodeId;         // Node ID of the other RFM board

  // Wait on an interrupt from the other Reflective Memory board
  if (m_adc->rfm()->driver().disableEvent(RFM2GEVENT_INTR3 )) 
	  return 1;
  if (m_adc->rfm()->driver().enableEvent( RFM2GEVENT_INTR3 ))
	  return 1;

  EventInfo.Event    = RFM2GEVENT_INTR3;  // We'll wait on this interrupt
  EventInfo.Timeout  = DAC_TIMEOUT;       // We'll wait this many milliseconds
  if (m_adc->rfm()->driver().waitForEvent( &EventInfo ))
	  return 1;
  otherNodeId = EventInfo.NodeId;
  RFM2G_UINT32 rfm2gCtrlSeq   = EventInfo.ExtendedInfo;
  RFM2G_UINT32 rfm2gMemNumber = rfm2gCtrlSeq & 0x000ffff;

  cout<<"rfm2    "<<rfm2gMemNumber<<endl;
  cout<<"loopPos "<<m_adc->status().loopPos<<endl;

 // t_dac_write.clock();

  // Now read data from other mBox
  RFM2G_UINT32 threshold = 0;
  // see if DMA threshold and buffer are intialized
  m_rfm->driver().getDMAThreshold(&threshold );

  int data_size = DAC_BUFFER_SIZE*sizeof(RFM2G_UINT32);
  RFM2G_UINT32* dacb=DAC_Buffer;
  if (status.loopPos%2==0) {
    dacb=DAC_Buffer2;
  }

  if (data_size<threshold) {
    // use PIO transfer
    if (RFM2gRead( m_adc->rfm()->handle(), DAC_MEMPOS + (rfm2gMemNumber * data_size), 
		   (void *)dacb, data_size )) return 1;
  }
  else {
    if (RFM2gRead( m_adc->rfm()->handle(), DAC_MEMPOS + (rfm2gMemNumber * data_size), 
		   (void *)pDmaMemory, data_size )) return 1;
    RFM2G_UINT32 * src = (RFM2G_UINT32*)pDmaMemory; 
    for (int i=0;i<DAC_BUFFER_SIZE;++i) {
      dacb[i]=src[i];
    }
  }
*/
 // t_dac_stop.clock();    

  /*
    cout << "RawOrbit X: "  << "\n";
    for (int j=0;j<8;++j) {
    for (int i=0;i<16;++i) {
    cout << setw(12) << i+16*j;
    }
    cout << "\n";
    for (int i=0;i<16;++i) {
    if ((i+16*j)<numBPMx) {
    cout << setw(12) << rADCdataX(i+16*j);
    }
    }
    cout << "\n";
    }
    //cout << "RawOrbit Y: " << rADCdataY << "\n";
    */


  /*
    cout << "DAC_Buffer = written by MATLAB mBox\n" << "DACout = calced changes\n";
    cout << "DAC_Buffer\n" << "DACout\n" << "DAC_Buffer-DACBuffer2\n" << "DACout -DACout2\n";
    if (status.loopPos%2==0) {
    for (int j=0;j<8;++j) {
    for (int i=0;i<16;++i) {
    //char corPos = DAC_WaveIndexX[0];
    cout << setw(11) << i+16*j;
    }
    cout<<"\n";
    for (int i=0;i<16;++i) {
    cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 )<< (DAC_Buffer[i+16*j]-halfDigits)/scaleDigitsX(0);
    }
    cout<<"\n";
      
    for (int i=0;i<16;++i) {
    cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 )<< (DACout[i+16*j]-halfDigits)/scaleDigitsX(0);
    }
    cout<<"\n";

    for (int i=0;i<16;++i) {
    cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 ) << (double(DAC_Buffer[i+16*j])-DAC_Buffer2[i+16*j])/scaleDigitsX(0);
    }
    cout<<"\n";
      

    for (int i=0;i<16;++i) {
    cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 ) << (double(DACout[i+16*j])-DACout2[i+16*j])/scaleDigitsX(0) ;
    }
    cout<<"\n";
    }
    }
  */

  return 0;
}
	

void CorrectionProcessor::calcSmat(const arma::mat &Smat,
                                   double Ivec,
                                   arma::vec &CMWeight,
                                   arma::mat &SmatInv)
{
    std::cout << "Calculate Smat" << std::endl;
    arma::mat U, S, V;
    arma::vec s;
    
    std::cout << "\tGiven : " << " Smat cols: " << Smat.n_cols << " smat rows " << Smat.n_rows << "  Ivec : " << Ivec << std::endl;
    arma::mat Smat_w = arma::zeros(Smat.n_rows, Smat.n_cols);
    std::cout << "\tmake Ivec" << std::endl;
    if (Ivec > Smat.n_rows) {
        std::cout << "\tIVec > Smat.n_rows: Setting Ivec = Smat.n_rows" << std::endl;
        double Ivec = Smat.n_rows;
    }
    if (m_useCMWeight) {
        std::cout << "\tcalc CMWeights" << std::endl;
        CMWeight = 1/(arma::trans(stddev(Smat)));
        std::cout << "\tInclude CMWeightX in SMat" << std::endl;
        for (int i = 0; i < Smat.n_cols; i++) {
            Smat_w.col(i) = Smat.col(i) * CMWeight(i);
        }
    } else {
        Smat_w = Smat;
    }
    std::cout << "\tcalc SVD" << std::endl;
    arma::svd(U,s,V,Smat_w);
    std::cout << "\treduce U to Ivec" << std::endl;
    U = U.cols(0,Ivec-1);
    std::cout << "\tTranspose U" << std::endl;
    U = trans(U);
    std::cout << "\tGet Diag Matrix of  S" << std::endl;
    S = diagmat(s.subvec(0,Ivec-1)); 
    std::cout << "\treduce V to Ivec" << std::endl;
    V = V.cols(0,Ivec-1);
    std::cout << "\tCalc new Matrix" << std::endl;
    SmatInv = V * arma::inv(S) * U;

    std::cout << "SVD complete ..." << std::endl;
}