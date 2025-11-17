#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>
#include <QLabel>
#include <QStackedWidget>

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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();
    void updateTimer();
    void on_startButton_clicked();
    void on_backButton_clicked();
    void on_noAppleGoalCheckBox_toggled(bool checked);
    void on_noTimerCheckBox_toggled(bool checked);

private:
    void initGame();
    void generateApple();
    void moveSnake();
    void updateGrid();
    void clearGrid();
    QString getSnakeColor(int segmentIndex);
    void checkWinCondition();
    void checkGameOver();
    void updateGameInfo(); // ДОБАВЛЕНО: объявление метода

    Ui::MainWindow *ui;

    static const int GRID_SIZE = 20;
    QLabel ***gridCells;

    // Таймеры
    QTimer *gameTimer;      // Для игрового цикла
    QTimer *countdownTimer; // Для обратного отсчета

    // Игровые переменные
    QVector<QPoint> snake;
    QPoint apple;
    QPoint direction;
    bool gameRunning;
    int score;
    int applesEaten;
    int timeLeft;

    // Настройки из главного меню
    int appleGoal;
    int initialTime;
    bool hasAppleGoal;
    bool hasTimer;
};

#endif
