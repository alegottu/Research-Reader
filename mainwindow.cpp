// NOTE: Code here adapated from piper-master

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "piper-master/piper/src/cpp/piper.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString qtext = ui->textEdit->toPlainText();
    string text = qtext.toStdString();

    piper::PiperConfig piperConfig{"", false, false};
    piper::SynthesisResult result;
    mutex mutAudio;
    condition_variable cvAudio;
    bool audioReady = false;
    bool audioFinished = false;
    vector<int16_t> audioBuffer;
    vector<int16_t> sharedAudioBuffer;

#ifdef _WIN32
    // Needed on Windows to avoid terminal conversions
    setmode(fileno(stdout), O_BINARY);
    setmode(fileno(stdin), O_BINARY);
#endif

    thread rawOutputThread(rawOutputProc, ref(sharedAudioBuffer),
                           ref(mutAudio), ref(cvAudio), ref(audioReady),
                           ref(audioFinished));
    auto audioCallback = [&audioBuffer, &sharedAudioBuffer, &mutAudio,
                          &cvAudio, &audioReady]() {
        // Signal thread that audio is ready
        {
            unique_lock lockAudio(mutAudio);
            copy(audioBuffer.begin(), audioBuffer.end(),
                 back_inserter(sharedAudioBuffer));
            cout << sharedAudioBuffer[sharedAudioBuffer.size()-1] << endl;
            audioReady = true;
            cvAudio.notify_one();
        }
    };
    piper::textToAudio(piperConfig, voice, text, audioBuffer, result,
                       audioCallback);

    // Signal thread that there is no more audio
    {
        unique_lock lockAudio(mutAudio);
        audioReady = true;
        audioFinished = true;
        cvAudio.notify_one();
    }

    // Wait for audio output to finish
    rawOutputThread.join();
}
