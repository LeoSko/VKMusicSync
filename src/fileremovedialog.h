#ifndef FILEREMOVEDIALOG_H
#define FILEREMOVEDIALOG_H

#include <QDialog>

namespace Ui {
class FileRemoveDialog;
}

class FileRemoveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileRemoveDialog(QWidget *parent = 0);
    void setFileList(const QStringList &list);
    ~FileRemoveDialog();

private slots:

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::FileRemoveDialog *m_ui;
};

#endif // FILEREMOVEDIALOG_H
