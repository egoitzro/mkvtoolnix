#include "common/common_pch.h"

#include <QAction>
#include <QMenu>

#include "common/qt.h"
#include "mkvtoolnix-gui/forms/main_window/status_bar_progress_widget.h"
#include "mkvtoolnix-gui/jobs/tool.h"
#include "mkvtoolnix-gui/main_window/main_window.h"
#include "mkvtoolnix-gui/main_window/status_bar_progress_widget.h"

namespace mtx { namespace gui {

StatusBarProgressWidget::StatusBarProgressWidget(QWidget *parent)
  : QWidget{parent}
  , ui{new Ui::StatusBarProgressWidget}
{
  ui->setupUi(this);

  m_pixmaps << QPixmap{Q(":/icons/16x16/dialog-warning.png")};
  m_pixmaps << QPixmap{Q(":/icons/16x16/dialog-warning-grayscale.png")};
  m_pixmaps << QPixmap{Q(":/icons/16x16/dialog-error.png")};
  m_pixmaps << QPixmap{Q(":/icons/16x16/dialog-error-grayscale.png")};

  m_timer.setInterval(1000);

  connect(&m_timer, &QTimer::timeout, this, &StatusBarProgressWidget::updateWarningsAndErrorsIcons);

  connect(ui->warningsContainer, &QWidget::customContextMenuRequested, this, &StatusBarProgressWidget::showContextMenu);
  connect(ui->errorsContainer,   &QWidget::customContextMenuRequested, this, &StatusBarProgressWidget::showContextMenu);
}

StatusBarProgressWidget::~StatusBarProgressWidget() {
}

void
StatusBarProgressWidget::reset() {
  setProgress(0, 0);
}

void
StatusBarProgressWidget::setProgress(int progress,
                                     int totalProgress) {
  ui->progress->setValue(progress);
  ui->totalProgress->setValue(totalProgress);
}

void
StatusBarProgressWidget::setJobStats(int numPendingAuto,
                                     int numPendingManual,
                                     int numRunning,
                                     int) {
  m_numPendingAuto   = numPendingAuto;
  m_numPendingManual = numPendingManual;
  m_numRunning       = numRunning;

  setLabelTexts();
}

void
StatusBarProgressWidget::setNumUnacknowledgedWarningsOrErrors(int numWarnings,
                                                              int numErrors) {
  m_numWarnings = numWarnings;
  m_numErrors   = numErrors;

  auto isActive = m_timer.isActive();

  if (!isActive && (m_numWarnings || m_numErrors)) {
    m_timerStep = 0;
    m_timer.start();

  } else if (isActive && !m_numWarnings && !m_numErrors) {
    m_timer.stop();
    updateWarningsAndErrorsIcons();
  }

  setLabelTexts();
}

void
StatusBarProgressWidget::retranslateUi() {
  ui->retranslateUi(this);

  setLabelTexts();
}

void
StatusBarProgressWidget::setLabelTexts() {
  ui->numJobsLabel->setText(QY("%1 automatic, %2 manual, %3 running").arg(m_numPendingAuto).arg(m_numPendingManual).arg(m_numRunning));
  ui->warningsLabel->setText(QNY("%1 warning", "%1 warnings", m_numWarnings).arg(m_numWarnings));
  ui->errorsLabel  ->setText(QNY("%1 error",   "%1 errors",   m_numErrors)  .arg(m_numErrors));
}

void
StatusBarProgressWidget::updateWarningsAndErrorsIcons() {
  auto warningOffset = !m_numWarnings || !(m_timerStep % 2) ? 1 : 0;
  auto errorOffset   = !m_numErrors   || !(m_timerStep % 2) ? 1 : 0;

  ui->warningsIconLabel->setPixmap(m_pixmaps[0 + warningOffset]);
  ui->errorsIconLabel  ->setPixmap(m_pixmaps[2 + errorOffset]);

  ++m_timerStep;
}

void
StatusBarProgressWidget::showContextMenu(QPoint const &pos) {
  QMenu menu{this};

  auto acknowledgeWarnings = new QAction{&menu};
  auto acknowledgeErrors   = new QAction{&menu};

  acknowledgeWarnings->setText(QY("Acknowledge all &warnings"));
  acknowledgeErrors->setText(QY("Acknowledge all &errors"));

  acknowledgeWarnings->setEnabled(!!m_numWarnings);
  acknowledgeErrors->setEnabled(!!m_numErrors);

  connect(acknowledgeWarnings, &QAction::triggered, MainWindow::jobTool()->model(), &mtx::gui::Jobs::Model::acknowledgeAllWarnings);
  connect(acknowledgeErrors,   &QAction::triggered, MainWindow::jobTool()->model(), &mtx::gui::Jobs::Model::acknowledgeAllErrors);

  menu.addAction(acknowledgeWarnings);
  menu.addAction(acknowledgeErrors);

  menu.exec(static_cast<QWidget *>(sender())->mapToGlobal(pos));
}

}}
