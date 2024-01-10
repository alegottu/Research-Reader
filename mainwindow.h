#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "piper-master/src/cpp/piper.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void SetVoice(const piper::Voice& voice);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    piper::Voice voice;
};
#endif // MAINWINDOW_H
