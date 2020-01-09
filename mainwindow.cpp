#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QDesktopServices"
#include "algorithm"
#include "iostream"
#include "string"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->geometry().width(),this->geometry().height());

    updateList();

    connect(updater, SIGNAL(timeout()), this, SLOT(update()));
    player->setVolume(100);

    ui->listWidget->setCurrentRow(0);
    if(ui->listWidget->count() != 0){
        loadTrack();
        player->pause();
        updater->start();

    }
    setWindowIcon(QIcon(":/Playericons/icons8-music-record-64.png"));
    ui->searchBar->setPlaceholderText("Search your Song");

    //Icon
    ui->play->setIcon(QIcon(":/Playericons/icons8-play-30.png"));
    ui->back->setIcon(QIcon(":/Playericons/icons8-less-than-30.png"));
    ui->forward->setIcon(QIcon(":/Playericons/icons8-more-than-30.png"));
    ui->shuffle->setIcon(QIcon(":/Playericons/icons8-shuffle-30.png"));
    ui->mute->setIcon(QIcon(":/Playericons/icons8-audio-30.png"));
    ui->repeat->setIcon(QIcon(":/Playericons/icons8-repeat-30.png"));
    ui->save->setIcon(QIcon(":/Playericons/icons8-save-30.png"));
    ui->remove->setIcon(QIcon(":/Playericons/icons8-minus-30.png"));
    ui->add->setIcon(QIcon(":/Playericons/icons8-plus-30 (1).png"));

    ui->play->setCheckable(true);
    ui->mute->setCheckable(true);

    //Font Configurations
    ui->currentSong->setFont(QFont(":/Fonts/Rubik-Black.ttf"));
    ui->Menu->setFont(QFont(":/Fonts/Rubik-Black.ttf"));
    ui->listWidget->setFont(QFont(":/Fonts/Rubik-Black.ttf"));
    ui->searchBar->setFont(QFont(":/Fonts/Rubik-Black.ttf"));

}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_add_clicked()
{   bool startUpdater = false;if(ui->listWidget->count() == 0) startUpdater = true;
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"));
    if(!files.empty())
    {
        playlist.add(files);
        updateList();
        ui->save->setChecked(false);
        if(shuffle) shufflePlaylist();
        if(startUpdater) updater->start();
    }
}

void MainWindow::on_remove_clicked()
{
    int index = getIndex();
    if(index != -1)
    {
       playlist.remove(index);
       updateList();
       ui->listWidget->setCurrentRow(index);
       ui->save->setChecked(false);
       if(shuffle) shufflePlaylist();
    }
}

void MainWindow::on_save_clicked()
{
    playlist.save();
    ui->save->setChecked(true);

}


void MainWindow::on_listWidget_doubleClicked(const QModelIndex &)
{
    lCounter = getIndex();
    ui->play->setChecked(false);
    ui->searchBar->clear();

    loadTrack();
    player->play();
}


void MainWindow::on_play_clicked()
{
    if(ui->listWidget->count() != 0)
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->pause();
    }
   else
   {
        player->play();
        updater->start();
   }
}

void MainWindow::on_forward_clicked()
{   if(ui->listWidget->count() != 0)
    if(repeat)
    {
        repeat = !repeat;next();repeat = !repeat;
    }
    else
    {
        next();
    }
}

void MainWindow::on_back_clicked()
{   if(ui->listWidget->count() != 0)
    if(player->position() > 3000)
    {
       player->setPosition(0);
    }
    else
    {
        back();
    }
}

void MainWindow::on_volumeBar_valueChanged(int value)
{
    player->setVolume(value);
    (value != 0)?ui->mute->setChecked(false):ui->mute->setChecked(true);
}

void MainWindow::on_seekBar_sliderMoved(int position)
{
    player->setPosition(player->duration() / 1000 * position);
}

void MainWindow::on_mute_clicked()
{
    muted = !muted;
    (muted)?player->setVolume(0):player->setVolume(ui->volumeBar->value());
}


void MainWindow::on_repeat_clicked()
{
     repeat = !repeat;
}


void MainWindow::on_shuffle_clicked()
{
    shuffle = !shuffle;
    if(shuffle)
        shufflePlaylist();
}

void MainWindow::update()
{   if(!ui->seekBar->isSliderDown())
        ui->seekBar->setValue((double)player->position()/player->duration() * 1000);

    if(player->state() == QMediaPlayer::StoppedState)
    {
        next();
    }
}

void MainWindow::updateList()
{
    ui->listWidget->clear();
    ui->listWidget->addItems(playlist.getTracksNameList());
}

int MainWindow::getIndex()
{
    return ui->listWidget->currentIndex().row();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return :
    {
        lCounter = getIndex();
        if(lCounter != -1)
        {
            ui->play->setChecked(false);
            ui->searchBar->clear();

           loadTrack();
           player->play();
        }
        break;
    }
    case Qt::Key_Up :
    {
        int ind = getIndex() - 1;if(ind < 0)ind = ui->listWidget->count() - 1;
        ui->listWidget->setCurrentRow(ind);
        break;
    }
    case Qt::Key_Down :
    {
        int ind = getIndex() + 1;if(ind >= ui->listWidget->count())ind = 0;
        ui->listWidget->setCurrentRow(ind);
        break;
    }
    default :
    {
        ui->searchBar->setFocus();

        break;
    }
    }
}

void MainWindow::next()
{
    lCounter++;

    if(repeat)
    {
        lCounter--;
    }

    if(lCounter >= ui->listWidget->count())
        lCounter = 0;

    (!shuffle or repeat) ? ui->listWidget->setCurrentRow(lCounter) : ui->listWidget->setCurrentRow(shuffledPlaylist[lCounter]);

    ui->play->setChecked(false);
    ui->searchBar->clear();

    loadTrack();
    player->play();

}

void MainWindow::back()
{
     lCounter--;

     if(lCounter < 0)
        lCounter = ui->listWidget->count() - 1;


     (!shuffle) ? ui->listWidget->setCurrentRow(lCounter) : ui->listWidget->setCurrentRow(shuffledPlaylist[lCounter]);

     ui->play->setChecked(false);
     ui->searchBar->clear();

     loadTrack();
     player->play();
}

void MainWindow::shufflePlaylist()
{
    shuffledPlaylist.resize(0);

    for(int i = 0; i < ui->listWidget->count(); i++)
    {
        shuffledPlaylist.push_back(i);
    }

    random_shuffle(shuffledPlaylist.begin(), shuffledPlaylist.end());
}

void MainWindow::loadTrack()
{
     QString qstr = QString::fromStdString(playlist.tracks[getIndex()].getLocation());
     player->setMedia(QUrl::fromLocalFile(qstr));
     qstr = QString::fromStdString(playlist.tracks[getIndex()].getName());
     ui->currentSong->setText(qstr);
}

void MainWindow::on_searchBar_textChanged(const QString &arg1)
{
    if(ui->searchBar->text().toStdString() != "")
    for(int i = 0; i < ui->listWidget->count(); i++)
    {
        if(ui->listWidget->item(i)->text().toLower().toStdString().find(arg1.toLower().toStdString()) != string::npos )
        {
            ui->listWidget->setCurrentRow(i);
            break;
        }
    }

}

void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_play_toggled(bool checked)
{
    if(checked)
    {
        ui->play->setIcon(QIcon(":/Playericons/icons8-play-30.png"));
        //qDebug()<<"Button ON";
    }
    else
    {
        ui->play->setIcon(QIcon(":/Playericons/icons8-pause-30.png"));
    }
}

void MainWindow::on_mute_toggled(bool checked)
{
    if(checked)
    {
        ui->mute->setIcon(QIcon(":/Playericons/icons8-mute-30.png"));
    }
    else
    {
        ui->mute->setIcon(QIcon(":/Playericons/icons8-audio-30.png"));
    }
}

