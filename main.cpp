// NOTE: Code here adapted from piper-master

#include "mainwindow.h"
#include "piper-master/src/cpp/piper.hpp"

#include <optional>
#include <filesystem>

#include <QApplication>
#include <QLocale>
#include <QTranslator>

using namespace std;

enum OutputType { OUTPUT_FILE, OUTPUT_DIRECTORY, OUTPUT_STDOUT, OUTPUT_RAW };

struct RunConfig
{
    // Path to .onnx voice file
    filesystem::path modelPath;

    // Path to JSON voice config file
    filesystem::path modelConfigPath;

    // Type of output to produce.
    // Default is to write a WAV file in the current directory.
    OutputType outputType = OUTPUT_RAW;

    // Path for output
    optional<filesystem::path> outputPath = filesystem::path(".");

    // Numerical id of the default speaker (multi-speaker voices)
    optional<piper::SpeakerId> speakerId;

    // Amount of noise to add during audio generation
    optional<float> noiseScale;

    // Speed of speaking (1 = normal, < 1 is faster, > 1 is slower)
    optional<float> lengthScale;

    // Variation in phoneme lengths
    optional<float> noiseW;

    // Seconds of silence to add after each sentence
    optional<float> sentenceSilenceSeconds;

    // Path to espeak-ng data directory (default is next to piper executable)
    optional<filesystem::path> eSpeakDataPath;

    // Path to libtashkeel ort model
    // https://github.com/mush42/libtashkeel/
    optional<filesystem::path> tashkeelModelPath;

    // stdin input is lines of JSON instead of text with format:
    // {
    //   "text": str,               (required)
    //   "speaker_id": int,         (optional)
    //   "speaker": str,            (optional)
    //   "output_file": str,        (optional)
    // }
    bool jsonInput = false;

    // Seconds of extra silence to insert after a single phoneme
    optional<std::map<piper::Phoneme, float>> phonemeSilenceSeconds;

    // true to use CUDA execution provider
    bool useCuda = false;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();

    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "PiperTTS-front_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    // Piper setup start

    filesystem::path model{"piper-master/en_US-lessac-medium.onnx"};
    filesystem::path modelConfig{"piper-master/en_US-lessac-medium.onnx.json"};
    RunConfig runConfig{model, modelConfig}; // Later get args from settings file which is edited by menu
    piper::PiperConfig piperConfig{"", false, false};
    piper::Voice voice;
    piper::loadVoice(piperConfig, runConfig.modelPath.string(),
                runConfig.modelConfigPath.string(), voice, runConfig.speakerId,
                runConfig.useCuda);
    w.SetVoice(voice);
    piper::initialize(piperConfig);

    // Scales
    if (runConfig.noiseScale) {
        voice.synthesisConfig.noiseScale = runConfig.noiseScale.value();
    }

    if (runConfig.lengthScale) {
        voice.synthesisConfig.lengthScale = runConfig.lengthScale.value();
    }

    if (runConfig.noiseW) {
        voice.synthesisConfig.noiseW = runConfig.noiseW.value();
    }

    if (runConfig.sentenceSilenceSeconds) {
        voice.synthesisConfig.sentenceSilenceSeconds =
            runConfig.sentenceSilenceSeconds.value();
    }

    if (runConfig.phonemeSilenceSeconds) {
        if (!voice.synthesisConfig.phonemeSilenceSeconds) {
            // Overwrite
            voice.synthesisConfig.phonemeSilenceSeconds =
                runConfig.phonemeSilenceSeconds;
        } else {
            // Merge
            for (const auto &[phoneme, silenceSeconds] :
                 *runConfig.phonemeSilenceSeconds) {
                voice.synthesisConfig.phonemeSilenceSeconds->try_emplace(
                    phoneme, silenceSeconds);
            }
        }

    } // if phonemeSilenceSeconds
    // Piper setup end

    return a.exec();
}
