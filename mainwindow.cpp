#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gameRunning(false)
    , score(0)
    , applesEaten(0)
    , timeLeft(0)
    , appleGoal(10)
    , initialTime(60)
    , hasAppleGoal(false)
    , hasTimer(false)
{
    ui->setupUi(this);

    //2D массив для клеток сетки
    gridCells = new QLabel**[GRID_SIZE];
    for (int i = 0; i < GRID_SIZE; ++i) {
        gridCells[i] = new QLabel*[GRID_SIZE];
        for (int j = 0; j < GRID_SIZE; ++j) {
            QLabel *cell = new QLabel();
            cell->setFixedSize(25, 25);
            cell->setStyleSheet("background-color: black; border: 1px solid #333;");
            ui->gridLayout->addWidget(cell, i, j);
            gridCells[i][j] = cell;
        }
    }

    //таймер
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);

    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &MainWindow::updateTimer);

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &MainWindow::on_backButton_clicked);
    connect(ui->noAppleGoalCheckBox, &QCheckBox::toggled, this, &MainWindow::on_noAppleGoalCheckBox_toggled);
    connect(ui->noTimerCheckBox, &QCheckBox::toggled, this, &MainWindow::on_noTimerCheckBox_toggled);

    initGame();
}

MainWindow::~MainWindow()
{
    //сброс поля
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            delete gridCells[i][j];
        }
        delete[] gridCells[i];
    }
    delete[] gridCells;

    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    appleGoal = ui->appleGoalSpinBox->value();
    initialTime = ui->timerSpinBox->value();
    hasAppleGoal = !ui->noAppleGoalCheckBox->isChecked();
    hasTimer = !ui->noTimerCheckBox->isChecked();

    ui->stackedWidget->setCurrentIndex(1);

    initGame();
}

void MainWindow::on_backButton_clicked()
{
    gameTimer->stop();
    countdownTimer->stop();
    gameRunning = false;

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_noAppleGoalCheckBox_toggled(bool checked)
{
    ui->appleGoalSpinBox->setEnabled(!checked);
}

void MainWindow::on_noTimerCheckBox_toggled(bool checked)
{
    ui->timerSpinBox->setEnabled(!checked);
}

void MainWindow::updateTimer()
{
    if (timeLeft > 0) {
        timeLeft--;
        // Обновляем отображение времени в infoLabel
        updateGameInfo();
    }

    if (timeLeft <= 0 && hasTimer) {
        checkGameOver();
    }
}

//Обновление инфы в игровом окне
void MainWindow::updateGameInfo()
{
    QString infoText;

    if (!gameRunning) {
        infoText = "Press K to start the game. Use WASD to move.";
    } else {
        infoText = "Game Running! Use WASD to control the snake.";
    }

    // Добавление инфы о времени и яблоках
    if (hasTimer) {
        infoText += QString(" Time: %1s").arg(timeLeft);
    }

    if (hasAppleGoal) {
        infoText += QString(" Apples: %1/%2").arg(applesEaten).arg(appleGoal);
    }

    ui->infoLabel->setText(infoText);
}

QString MainWindow::getSnakeColor(int segmentIndex)
{
    int totalSegments = snake.size();
    if (totalSegments <= 1) {
        return "#006400";
    }

    float progress = static_cast<float>(segmentIndex) / (totalSegments - 1);

    int red, green, blue;

    if (progress < 0.5) {
        progress = progress * 2;
        red = 0 + static_cast<int>(50 * progress);
        green = 100 + static_cast<int>(105 * progress);
        blue = 0 + static_cast<int>(50 * progress);
    } else {
        progress = (progress - 0.5) * 2;
        red = 50 + static_cast<int>(64 * progress);
        green = 205 + static_cast<int>(49 * progress);
        blue = 50 + static_cast<int>(64 * progress);
    }

    red = qMin(255, qMax(0, red));
    green = qMin(255, qMax(0, green));
    blue = qMin(255, qMax(0, blue));

    return QString("rgb(%1, %2, %3)").arg(red).arg(green).arg(blue);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (ui->stackedWidget->currentIndex() != 1) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    // ИГНОРИРУЕМ клавишу K если игра не запущена и мы на игровой странице
    if (!gameRunning) {
        if (event->key() == Qt::Key_K) {
            gameRunning = true;
            gameTimer->start(150);
            if (hasTimer) {
                countdownTimer->start(1000);
            }
            updateGameInfo();
            event->accept();
            return;
        }
        event->accept();
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
        if (direction != QPoint(0, 1))
            direction = QPoint(0, -1);
        break;
    case Qt::Key_S:
        if (direction != QPoint(0, -1))
            direction = QPoint(0, 1);
        break;
    case Qt::Key_A:
        if (direction != QPoint(1, 0))
            direction = QPoint(-1, 0);
        break;
    case Qt::Key_D:
        if (direction != QPoint(-1, 0))
            direction = QPoint(1, 0);
        break;
    case Qt::Key_K:
        // Пауза/продолжение игры по клавише K
        if (gameRunning) {
            gameRunning = false;
            gameTimer->stop();
            if (hasTimer) {
                countdownTimer->stop();
            }
            updateGameInfo();
        } else {
            gameRunning = true;
            gameTimer->start(150);
            if (hasTimer) {
                countdownTimer->start(1000);
            }
            updateGameInfo();
        }
        break;
    }

    event->accept();
}

void MainWindow::gameLoop()
{
    if (!gameRunning) return;

    moveSnake();
    updateGrid();
    checkWinCondition();
}

void MainWindow::initGame()
{
    snake.clear();

    // Начальная позиция змейки (3 сегмента)
    snake.append(QPoint(GRID_SIZE / 2, GRID_SIZE / 2));
    snake.append(QPoint(GRID_SIZE / 2 - 1, GRID_SIZE / 2));
    snake.append(QPoint(GRID_SIZE / 2 - 2, GRID_SIZE / 2));

    direction = QPoint(1, 0);
    score = 0;
    applesEaten = 0;
    timeLeft = initialTime;

    generateApple();
    updateGrid();

    ui->scoreLabel->setText(QString("Score: %1").arg(score));
    updateGameInfo();

    gameRunning = false;
}

void MainWindow::generateApple()
{
    do {
        apple.setX(QRandomGenerator::global()->bounded(GRID_SIZE));
        apple.setY(QRandomGenerator::global()->bounded(GRID_SIZE));
    } while (snake.contains(apple));
}

void MainWindow::moveSnake()
{
    QPoint newHead = snake.first() + direction;

    //телепортация
    if (newHead.x() < 0) newHead.setX(GRID_SIZE - 1);
    if (newHead.x() >= GRID_SIZE) newHead.setX(0);
    if (newHead.y() < 0) newHead.setY(GRID_SIZE - 1);
    if (newHead.y() >= GRID_SIZE) newHead.setY(0);

    //проверка столкновения с собой
    if (snake.contains(newHead)) {
        checkGameOver();
        return;
    }

    snake.prepend(newHead);

    if (newHead == apple) {
        score += 10;
        applesEaten++;

        if (hasTimer) {
            timeLeft += 10;
        }

        ui->scoreLabel->setText(QString("Score: %1").arg(score));
        updateGameInfo();

        generateApple();
    } else {
        snake.removeLast();
    }
}

void MainWindow::checkWinCondition()
{
    if (hasAppleGoal && applesEaten >= appleGoal) {
        gameRunning = false;
        gameTimer->stop();
        countdownTimer->stop();

        QMessageBox::information(this, "Победа!",
                               QString("Поздравляем! Вы собрали все яблоки!\n"
                                       "Счет: %1\n"
                                       "Время осталось: %2 сек").arg(score).arg(timeLeft));
        initGame();
    }
}

void MainWindow::checkGameOver()
{
    bool gameOver = false;
    QString message;

    if (hasTimer && timeLeft <= 0) {
        gameOver = true;
        message = "Время вышло!";
    } else if (snake.contains(snake.first() + direction)) {
        gameOver = true;
        message = "Вы врезались в себя!";
    }

    if (gameOver) {
        gameRunning = false;
        gameTimer->stop();
        countdownTimer->stop();

        QMessageBox::information(this, "Конец игры",
                               QString("%1\nСчет: %2\nЯблок собрано: %3").arg(message).arg(score).arg(applesEaten));
        initGame();
    }
}

void MainWindow::updateGrid()
{
    clearGrid();

    //градиент
    for (int i = 0; i < snake.size(); ++i) {
        const QPoint &segment = snake[i];
        int x = segment.x();
        int y = segment.y();
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            QString color = getSnakeColor(i);
            gridCells[y][x]->setStyleSheet(
                QString("background-color: %1; border: 1px solid #333;").arg(color)
            );
        }
    }

    if (apple.x() >= 0 && apple.x() < GRID_SIZE && apple.y() >= 0 && apple.y() < GRID_SIZE) {
        gridCells[apple.y()][apple.x()]->setStyleSheet("background-color: red; border: 1px solid #333;");
    }
}

void MainWindow::clearGrid()
{
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            gridCells[i][j]->setStyleSheet("background-color: black; border: 1px solid #333;");
        }
    }
}
