#include "fileremovedialog.h"
#include "ui_fileremovedialog.h"
#include "stringconstants.h"
#include <QMessageBox>

FileRemoveDialog::FileRemoveDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::FileRemoveDialog)
{
    m_ui->setupUi(this);
    this->setWindowTitle(FILE_DELETION_DIALOG_TITLE);
}

void FileRemoveDialog::setFileList(const QStringList &list)
{
    for (QString s : list)
    {
        m_ui->fileList->addItem(s);
    }
}

FileRemoveDialog::~FileRemoveDialog()
{
    delete m_ui;
}

void FileRemoveDialog::on_buttonBox_accepted()
{
    this->accept();
}

void FileRemoveDialog::on_buttonBox_rejected()
{
    this->reject();
}
