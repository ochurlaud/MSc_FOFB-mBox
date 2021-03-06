/*
    Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ADC_H
#define ADC_H

#include "define.h"
#include <vector>

class RFMDriver;
class DMA;

/**
 * @brief Read the data (= BPM values) from the RFM.
 *
 * It must first be asked to read, which copies the data to a local buffer,
 * then use the buffer.
 *
 * \code{.cpp}
 * // Initialize
 * ADC adc(driver, dma);
 * adc.init();
 *
 * // Then each time needed
 * adc.read();
 * RFM2G_INT16 value = adc.bufferAt(12); // To get the 12th element
 * std::vector<RFM2G_INT16> buffer = adc.buffer() // To get the full buffer
 * \endcode
 */
class ADC
{
public:
    /**
     * @brief Constructor
     */
    explicit ADC(RFMDriver *driver, DMA *dma);

    /**
     * @brief Destructor
     */
    ~ADC();

    /**
     * @brief Initialize the ADC.
     * @return 1 if error, 0 if success
     */
    int init();

    /**
     * @brief Stop the ADC.
     *
     * This must be called when quitting the program, would it be a crash or a normal exit,
     * @return 1 if error, 0 if success
     */
    int stop();

    /**
     * @brief Read the RFM
     *
     * First wait for an interruption from the RFM, then read the RFM into `m_buffer`.
     */
    int read();

    /**
     * @brief Access to an element of the buffer.
     *
     * @param id Index of the buffer element to return
     * @return Buffer element
     */
    const RFM2G_INT16 bufferAt(int id) const { if (id < m_buffer.size()) return m_buffer.at(id); };

    /**
     * @brief Return the full buffer vector.
     *
     * @return The buffer vector
     */
    const std::vector<RFM2G_INT16>& buffer() const { return m_buffer; };

        /**
     * @brief Getter for m_waveIndexX element.
     *
     * @param id Id of which we want the position.
     *
     * @return position in the RFM of the requested element.
     */
    double waveIndexXAt(int id) const { return m_waveIndexX.at(id); };

    /**
     * @brief Getter for m_waveIndexY element.
     *
     * @param id Id of which we want the position.
     *
     * @return position in the RFM of the requested element.
     */
    double waveIndexYAt(int id) const { return m_waveIndexY.at(id); };

    /**
     * @brief Setter for m_waveIndexX element.
     *
     * @param vect Vector to copy and save.
     */
    void setWaveIndexX(std::vector<double> vect) { m_waveIndexX = vect; };

    /**
     * @brief Setter for m_waveIndexY element.
     *
     * @param vect Vector to copy and save.
     */
    void setWaveIndexY(std::vector<double> vect) { m_waveIndexY = vect; };

private:

    /**
     * @brief Procedure to get the authorization to read the RFM.
     *
     * @param eventInfo Information about the event
     * @return Status of the function call
     */
    RFM2G_STATUS waitForEvent(RFM2GEVENTINFO &eventInfo);

    /**
     * @brief Pointer to a DMA object.
     */
    DMA *m_dma;

    /**
     * @brief Pointer to a RFMDriver object.
     */
    RFMDriver *m_driver;

    /**
     * @brief Vector representing the data buffer
     */
    std::vector<RFM2G_INT16> m_buffer;

    /**
     * @brief Look-Up Table for indexes: m_waveIndexX[CMx_index] = position of in RFM.
     */
    std::vector<double> m_waveIndexX;

    /**
     * @brief Look-Up Table for indexes: m_waveIndexY[CMy_index] = position of in RFM.
     */
    std::vector<double> m_waveIndexY;

    /**
     * @brief RFM node with which to communicate.
     */
    int m_node;
};

#endif // ADC_H
