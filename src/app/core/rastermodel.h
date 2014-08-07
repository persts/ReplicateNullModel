/*
** Copyright (C) 2008 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2008-02-11
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
** This work was partially funded by the National Aeronautics and Space Administration
** under award(s) NNX09AK19G
**
** This work was partially funded by the National Science Foundation under Grant No.DEB-0641023
**
** Any opinions, findings and conclusions or recommendations expressed in this material are those
** of the author(s) and do not necessarily reflect the views of the National Science Foundation (NSF)
*/
#ifndef RASTERMODEL_H
#define RASTERMODEL_H

#include "mtrand.h"
#include "gradientshader.h"
#include "localitylist.h"


#include <gdal.h>

#include <QMap>
#include <QString>
#include <QPointF>
#include <QPoint>
#include <QImage>

/*
 * Provides access to any GDAL readable raster file.
 *
 * Note: this class was written quickly and will perform poorly with large datasets
 * there is plenty of room for imporvement or reimplementation at a later date
 */

class RasterModel
{

  public:
    /** \brief Constructor */
    RasterModel();

    /** \brief Constructor */
    explicit RasterModel( QString );

    /** \brief Constructor */
    explicit RasterModel( QString, const GradientShader &theShader );

    /** \brief Deconstructor */
    ~RasterModel();

    /** \brief Model response ( pixel type ) */
    enum ModelResponse
    {
      Negative,
      Positive,
      NoData,
      OutOfBounds,
      RawData
    };

    /** \brief Generaters a random set of pixel coordinates from the predicted distribtion */
    QList<QPoint> generateRandomDistributionOfPoints( int theSamples, bool withReplacement = true );

    /** \brief The raster file name for the currently loaded layer*/
    QString fileName( bool justFileName = true );

    /** \brief Returns the height of the currently loaded raster layer */
    int height() const { return cvYSize; }

    /** \brief Returns true of a raster layer has been sucessfully loaded */
    bool isValid() const { return cvIsValid; }

    /** \brief Returns true of a raster layer has been sucessfully loaded and a threshhold applied*/
    bool isThreshholdApplied() const { return cvIsThresholdApplied; }

    /** \brief Returns the last error message */
    QString lastError();

    //TODO: allow option to set which band should be loaded
    /** \brief Loads a raster layer, expects at least a file name. providing a threshhold value is optional */
    bool load( QString, bool estimateMinMax = true, bool useThreshold = false, double theThreshold = 0.0, bool inverseRelationship=false );

    /** \brief Returns maximum pixel value */
    double maximum() { return cvMaximum; }

    /** \brief Returns minimum pixel value */
    double minimum() { return cvMinimum; }

    int numberOfBands() { return cvNumberOfBands; }

    /** \brief Return the model response at pixel coordinate x, y */
    RasterModel::ModelResponse modelResponseAtPixelCoordinate( int, int ) const;

    /** \brief Return the model response at real world x, y */
    RasterModel::ModelResponse modelResponseAtRealWorldCoordinate( double, double );

    /** \brief Return the model response at real world x, y represented by a QPointF */
    RasterModel::ModelResponse modelResponseAtRealWorldCoordinate( const QPointF &thePoint );

    /** \brief Returns the proportion of area classified as "positive" in the predicted distirbution */
    double positiveResponseProbability();

    double noDataValue() { return cvNoDataValue; }

    QMap< QString, QString > omissionList( LocalityList theCalibrationPoints );

    QMap< QString, QString > omissionListFull( LocalityList theCalibrationPoints );

    /** \brief Translates pixel coordinates into real world coordinates */
    QPointF pixelToRealWorld( double, double );

    /** \brief Translates pixel coordinates into real world coordinates */
    QPointF pixelToRealWorld( const QPoint &thePoint );

    /** \brief Translates pixel coordinates into real world coordinates */
    QPointF pixelToRealWorld( const QPointF &thePoint );

    /** \brief Returns a random point from the list of positive and negative areas in the predicted distirbution */
    QPoint randomModelDataPoint();

    /** \brief Returns the number of positive model responses or -1 if invalid */
    int randomModelResponse( int theNumberOfSamples );

    /** \brief Returns the actual pixel value ( not model response ) at pixel coordinates x, y */
    double rawDataAtPixelCoordinate( int, int );

    /** \brief Returns the actual pixel value ( not model response ) at pixel coordinates x, y */
    double rawDataAtPixelCoordinate( const QPoint &thePoint );

    /** \brief Returns the actual pixel value ( not model response ) at real world coordinates x, y */
    double rawDataAtRealWorldCoordinate( double, double );

    /** \brief Returns the actual pixel value ( not model response ) at real world coordinates x, y, represented by a QPointF */
    double rawDataAtRealWorldCoordinate( const QPointF &thePoint );

    /** \brief Translates real world coordinates into pixel coordinates */
    QPointF realWorldToPixel( double, double );

    /** \brief Convience function, same as realWorldToPixel( double, double ) */
    QPointF realWorldToPixel( const QPointF &thePoint );

    /** \brief Reloads the currently loaded raster layer */
    bool reload();

    /** \brief Reloads the currently loaded raster layer but does so with the provided threshhold */
    bool reload( double, bool inverseRelationship=false );

    void setShader( const GradientShader &theShader ) { cvShader = theShader; }

    GradientShader* shader() { return &cvShader; }

    /** \brief The threshhold applied to the raster model */
    double threshold() { return cvThreshold; }

    /** \brief The image depiction of the modeled raster layer */
    const QImage* toImage() { return cvImage; }

    /** \brief Returns the width of the currently loaded raster layer */
    int width() const { return cvXSize; }

    double xPixelResolution() { return cvGeotransform[1]; }

    double yPixelResolution() { return cvGeotransform[5]; }

    bool operator==( const RasterModel &that );

    bool operator!=( const RasterModel &that );

  private:
    double cvNoDataValue;
    bool cvValidNoDataValue;
    bool cvIsValid;
    double cvMinimum;
    double cvMaximum;
    bool cvIsThresholdApplied;
    double cvThreshold;
    int cvXSize;
    int cvYSize;
    double  cvGeotransform[6];
    GDALRasterBandH cvRasterBand;
    GDALDatasetH cvGDALDataSet;
    int cvNumberOfBands;
    QString cvRasterFileName;

    int cvModelPositive;
    int cvModelNegative;
    QList<QPoint> cvModelDataPoints;

    QImage* cvImage;

    QString cvFileName;

    QString cvLastError;
    MTRand* cvRNG;

    GradientShader cvShader;


    void init();
    RasterModel& operator=( const RasterModel& theSource );

};
#endif
