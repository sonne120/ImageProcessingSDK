#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <ImageProcessor.h>

class QDoubleSpinBox;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void imageProcessed(const QImage& resultImage);

private slots:
    void loadImage();
    void processBlur();
    void processResize();
    void processGrayscale();
    void processSharpen();
    void processComposite();
    void onImageProcessed(const QImage& resultImage);


private:
    QLabel* imageLabel = nullptr;
    QPushButton* loadButton = nullptr;
    QPushButton* blurButton = nullptr;
    QPushButton* resizeButton = nullptr;
    QPushButton* grayscaleButton = nullptr;
    QPushButton* sharpenButton = nullptr;
    QPushButton* compositeButton = nullptr;
    QDoubleSpinBox* amountSpin = nullptr;
    QDoubleSpinBox* sigmaSpin = nullptr;
    QSpinBox* thresholdSpin = nullptr;
    QImage currentImage;

    std::unique_ptr<ip::ImageProcessor> processor;
};
