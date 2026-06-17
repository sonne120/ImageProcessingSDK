#include "mainwindow.h"
#include "QImageAdapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QDebug>
#include <chrono>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->resize(800, 600); 

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    imageLabel = new QLabel("No Image Loaded", centralWidget);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 200);
    imageLabel->setMaximumHeight(400);  
    imageLabel->setStyleSheet("QLabel { background-color : #f0f0f0; border: 1px solid #cccccc; }");

    loadButton = new QPushButton("Load Image", centralWidget);
    blurButton = new QPushButton("Apply Blur (r=20)", centralWidget);
    resizeButton = new QPushButton("Apply Resize (300x300)", centralWidget);
    grayscaleButton = new QPushButton("Apply Grayscale", centralWidget);

    // Custom sharpen (Unsharp Mask) controls: Amount / Radius / Threshold
    amountSpin = new QDoubleSpinBox(centralWidget);
    amountSpin->setRange(0.0, 10.0);
    amountSpin->setSingleStep(0.1);
    amountSpin->setDecimals(1);
    amountSpin->setValue(2.2);
    amountSpin->setPrefix("Amount: ");

    sigmaSpin = new QDoubleSpinBox(centralWidget);
    sigmaSpin->setRange(0.1, 50.0);
    sigmaSpin->setSingleStep(0.5);
    sigmaSpin->setDecimals(1);
    sigmaSpin->setValue(6.0);
    sigmaSpin->setPrefix("Radius: ");

    thresholdSpin = new QSpinBox(centralWidget);
    thresholdSpin->setRange(0, 255);
    thresholdSpin->setValue(12);
    thresholdSpin->setPrefix("Threshold: ");

    sharpenButton = new QPushButton("Apply Sharpen", centralWidget);

    compositeButton = new QPushButton("Apply Composite: Blur→Resize", centralWidget);


    blurButton->setEnabled(false);
    resizeButton->setEnabled(false);
    grayscaleButton->setEnabled(false);
    sharpenButton->setEnabled(false);
    compositeButton->setEnabled(false);

    QHBoxLayout* sharpenRow = new QHBoxLayout();
    sharpenRow->addWidget(amountSpin);
    sharpenRow->addWidget(sigmaSpin);
    sharpenRow->addWidget(thresholdSpin);

    layout->addWidget(imageLabel);
    layout->addWidget(loadButton);
    layout->addWidget(blurButton);
    layout->addWidget(resizeButton);
    layout->addWidget(grayscaleButton);
    layout->addLayout(sharpenRow);
    layout->addWidget(sharpenButton);
    layout->addWidget(compositeButton);

    setCentralWidget(centralWidget);
    setWindowTitle("ImageProcessingSDK Client");

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(blurButton, &QPushButton::clicked, this, &MainWindow::processBlur);
    connect(resizeButton, &QPushButton::clicked, this, &MainWindow::processResize);
    connect(grayscaleButton, &QPushButton::clicked, this, &MainWindow::processGrayscale);
    connect(sharpenButton, &QPushButton::clicked, this, &MainWindow::processSharpen);
    connect(compositeButton, &QPushButton::clicked, this, &MainWindow::processComposite);
    connect(this, &MainWindow::imageProcessed, this, &MainWindow::onImageProcessed, Qt::QueuedConnection);

    processor = std::make_unique<ip::ImageProcessor>();
}

MainWindow::~MainWindow() {}

void MainWindow::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)", nullptr, QFileDialog::DontUseNativeDialog);
    if (!fileName.isEmpty()) {
        if (!currentImage.load(fileName)) {
            QMessageBox::warning(this, "Error", "Failed to load image: " + fileName);
            return;
        }
        currentImage = currentImage.convertToFormat(QImage::Format_RGBA8888);

        imageLabel->clear();
        QPixmap pixmap = QPixmap::fromImage(currentImage);
        QPixmap scaledPixmap = pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(scaledPixmap);

        blurButton->setEnabled(true);
        resizeButton->setEnabled(true);
        grayscaleButton->setEnabled(true);
        sharpenButton->setEnabled(true);
        compositeButton->setEnabled(true);
    }
}

void MainWindow::processBlur()
{
    if (currentImage.isNull()) return;

    blurButton->setEnabled(false);
    imageLabel->setText("Applying Blur...");

    QImage snapshot = currentImage; 
    std::thread([this, snapshot]() {
        SDK_Image_t sdkImage = QtAdapter::qImageToSDK(snapshot);

        processor->processBlur(&sdkImage, 20, 4, 8);

        QImage result = QtAdapter::sdkToQImage(&sdkImage);

        if (sdkImage.data) {
            std::free(sdkImage.data);
        }
        sdkImage.data = nullptr;
        ipsdk_free_image(&sdkImage);

        QMetaObject::invokeMethod(this, [this, result]() {
            onImageProcessed(result);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::processResize()
{
    if (currentImage.isNull()) return;

    resizeButton->setEnabled(false);
    imageLabel->setText("Applying Resize...");

    QImage snapshot = currentImage;  
    std::thread([this, snapshot]() {
        SDK_Image_t sdkImage = QtAdapter::qImageToSDK(snapshot);

        processor->processResize(&sdkImage, 4, 8);

        QImage result = QtAdapter::sdkToQImage(&sdkImage);

        if (sdkImage.data) {
            std::free(sdkImage.data);
        }
        sdkImage.data = nullptr;
        ipsdk_free_image(&sdkImage);

        QMetaObject::invokeMethod(this, [this, result]() {
            onImageProcessed(result);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::processGrayscale()
{
    if (currentImage.isNull()) return;

    grayscaleButton->setEnabled(false);
    imageLabel->setText("Applying Grayscale...");

    QImage snapshot = currentImage;
    std::thread([this, snapshot]() {
        SDK_Image_t sdkImage = QtAdapter::qImageToSDK(snapshot);

        processor->processGrayscale(&sdkImage, 4, 8);

        QImage result = QtAdapter::sdkToQImage(&sdkImage);

        if (sdkImage.data) {
            std::free(sdkImage.data);
        }
        sdkImage.data = nullptr;
        ipsdk_free_image(&sdkImage);

        QMetaObject::invokeMethod(this, [this, result]() {
            onImageProcessed(result);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::processSharpen()
{
    if (currentImage.isNull()) return;

    const double amount = amountSpin->value();
    const double sigma = sigmaSpin->value();
    const int threshold = thresholdSpin->value();

    sharpenButton->setEnabled(false);
    imageLabel->setText("Applying Sharpen...");

    QImage snapshot = currentImage;
    std::thread([this, snapshot, amount, sigma, threshold]() {
        SDK_Image_t sdkImage = QtAdapter::qImageToSDK(snapshot);

        // 1x1 grid = whole image: the wide Gaussian would otherwise leave seams.
        processor->processSharpen(&sdkImage, amount, sigma, threshold, 1, 1);

        QImage result = QtAdapter::sdkToQImage(&sdkImage);

        if (sdkImage.data) {
            std::free(sdkImage.data);
        }
        sdkImage.data = nullptr;
        ipsdk_free_image(&sdkImage);

        QMetaObject::invokeMethod(this, [this, result]() {
            onImageProcessed(result);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::processComposite()
{
    if (currentImage.isNull()) return;

    compositeButton->setEnabled(false);
    imageLabel->setText("Applying Composite: Blur→Resize...");

    QImage snapshot = currentImage;  
    std::thread([this, snapshot]() {
        SDK_Image_t sdkImage = QtAdapter::qImageToSDK(snapshot);

        processor->process(&sdkImage, 4, 8);

        QImage result = QtAdapter::sdkToQImage(&sdkImage);

        if (sdkImage.data) {
            std::free(sdkImage.data);
        }
        sdkImage.data = nullptr;
        ipsdk_free_image(&sdkImage);

        QMetaObject::invokeMethod(this, [this, result]() {
            onImageProcessed(result);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::onImageProcessed(const QImage& resultImage)
{

    if (!resultImage.isNull()) {
        currentImage = resultImage;
        qDebug() << "Processed image:" << currentImage.width() << "x" << currentImage.height();

        QPixmap pixmap = QPixmap::fromImage(currentImage);

        QSize labelSize = imageLabel->size();

        QPixmap scaledPixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        imageLabel->setPixmap(scaledPixmap);
        imageLabel->setVisible(true);
        imageLabel->repaint();

        qDebug() << "Pixmap set successfully";
    } else {
        QMessageBox::warning(this, "Error", "Failed to process image.");
    }

    blurButton->setEnabled(true);
    resizeButton->setEnabled(true);
    grayscaleButton->setEnabled(true);
    sharpenButton->setEnabled(true);
    compositeButton->setEnabled(true);
}

