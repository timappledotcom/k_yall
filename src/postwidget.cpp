#include "postwidget.h"
#include "accountmanager.h"
#include "imageuploader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QListWidget>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QTimer>
#include <QDebug>
#include <KLocalizedString>

PostWidget::PostWidget(AccountManager *accountManager, QWidget *parent)
    : QDialog(parent)
    , m_accountManager(accountManager)
    , m_imageUploader(new ImageUploader(this))
    , m_postCompletionHandled(false)
    , m_totalAccountsToPost(0)
    , m_completedPosts(0)
{
    setWindowTitle(i18n("New Post - K, Y'all"));
    setModal(false);
    resize(500, 400);
    setAcceptDrops(true);
    
    setupUI();
    updateAccountCheckboxes();
    
    connect(m_imageUploader, &ImageUploader::uploadCompleted,
            this, &PostWidget::onPostCompleted);
    
    // Connect to account manager for post completion signals
    connect(m_accountManager, &AccountManager::postCompleted,
            this, &PostWidget::onPostCompleted);
}

void PostWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Post text area
    m_postText = new QTextEdit(this);
    m_postText->setPlaceholderText(i18n("What's on your mind?"));
    m_postText->setMaximumHeight(150);
    connect(m_postText, &QTextEdit::textChanged, this, &PostWidget::onTextChanged);
    
    m_charCountLabel = new QLabel(QString::number(MAX_CHARACTER_COUNT), this);
    m_charCountLabel->setAlignment(Qt::AlignRight);
    m_charCountLabel->setStyleSheet("color: green;");
    
    // Account selection
    m_accountsGroup = new QGroupBox(i18n("Post to:"), this);
    m_accountsLayout = new QVBoxLayout(m_accountsGroup);
    
    // Images section
    m_imagesGroup = new QGroupBox(i18n("Images"), this);
    m_imagesLayout = new QVBoxLayout(m_imagesGroup);
    
    m_imagesList = new QListWidget(this);
    m_imagesList->setMaximumHeight(100);
    
    QHBoxLayout *imageButtonsLayout = new QHBoxLayout();
    m_addImageButton = new QPushButton(i18n("Add Image"), this);
    m_removeImageButton = new QPushButton(i18n("Remove Selected"), this);
    m_removeImageButton->setEnabled(false);
    
    imageButtonsLayout->addWidget(m_addImageButton);
    imageButtonsLayout->addWidget(m_removeImageButton);
    imageButtonsLayout->addStretch();
    
    m_imagesLayout->addWidget(m_imagesList);
    m_imagesLayout->addLayout(imageButtonsLayout);
    
    // Progress and status
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setVisible(false);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    m_postButton = new QPushButton(i18n("Post"), this);
    m_postButton->setDefault(true);
    m_postButton->setEnabled(false);
    
    m_cancelButton = new QPushButton(i18n("Cancel"), this);
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_postButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    // Layout assembly
    m_mainLayout->addWidget(m_postText);
    m_mainLayout->addWidget(m_charCountLabel);
    m_mainLayout->addWidget(m_accountsGroup);
    m_mainLayout->addWidget(m_imagesGroup);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_postButton, &QPushButton::clicked, this, &PostWidget::onPostClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_addImageButton, &QPushButton::clicked, this, &PostWidget::onAddImageClicked);
    connect(m_removeImageButton, &QPushButton::clicked, this, &PostWidget::onRemoveImageClicked);
    connect(m_imagesList, &QListWidget::itemSelectionChanged, [this]() {
        m_removeImageButton->setEnabled(m_imagesList->currentItem() != nullptr);
    });
}

void PostWidget::updateAccountCheckboxes()
{
    // Clear existing checkboxes
    for (QCheckBox *checkbox : m_accountCheckboxes) {
        m_accountsLayout->removeWidget(checkbox);
        delete checkbox;
    }
    m_accountCheckboxes.clear();
    
    // Add checkboxes for each configured account
    const auto accounts = m_accountManager->getAllAccounts();
    for (const auto &account : accounts) {
        QCheckBox *checkbox = new QCheckBox(
            QString("%1 (%2)").arg(account.displayName, account.service), this);
        checkbox->setProperty("accountId", account.id);
        checkbox->setChecked(account.defaultForPosting);
        
        m_accountCheckboxes.append(checkbox);
        m_accountsLayout->addWidget(checkbox);
        
        connect(checkbox, &QCheckBox::toggled, this, &PostWidget::onAccountSelectionChanged);
    }
    
    if (m_accountCheckboxes.isEmpty()) {
        QLabel *noAccountsLabel = new QLabel(i18n("No accounts configured. Please add accounts in Settings."), this);
        noAccountsLabel->setStyleSheet("color: orange; font-style: italic;");
        m_accountsLayout->addWidget(noAccountsLabel);
    }
    
    onAccountSelectionChanged();
}

void PostWidget::onTextChanged()
{
    updateCharacterCount();
    onAccountSelectionChanged();
}

void PostWidget::updateCharacterCount()
{
    int charCount = m_postText->toPlainText().length();
    int remaining = MAX_CHARACTER_COUNT - charCount;
    
    m_charCountLabel->setText(QString::number(remaining));
    
    if (remaining < 0) {
        m_charCountLabel->setStyleSheet("color: red; font-weight: bold;");
    } else if (remaining < 20) {
        m_charCountLabel->setStyleSheet("color: orange; font-weight: bold;");
    } else {
        m_charCountLabel->setStyleSheet("color: green;");
    }
}

void PostWidget::onAccountSelectionChanged()
{
    bool hasSelectedAccounts = false;
    for (QCheckBox *checkbox : m_accountCheckboxes) {
        if (checkbox->isChecked()) {
            hasSelectedAccounts = true;
            break;
        }
    }
    
    bool hasText = !m_postText->toPlainText().trimmed().isEmpty();
    bool validCharCount = m_postText->toPlainText().length() <= MAX_CHARACTER_COUNT;
    
    m_postButton->setEnabled(hasSelectedAccounts && hasText && validCharCount);
}

void PostWidget::onAddImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        i18n("Select Image"), QString(),
        i18n("Image Files (*.png *.jpg *.jpeg *.gif *.webp)"));
    
    if (!fileName.isEmpty() && !m_imagePaths.contains(fileName)) {
        if (m_imagePaths.size() >= 4) {
            QMessageBox::warning(this, i18n("Too Many Images"),
                                i18n("You can attach a maximum of 4 images per post."));
            return;
        }
        
        m_imagePaths.append(fileName);
        QFileInfo fileInfo(fileName);
        m_imagesList->addItem(fileInfo.fileName());
    }
}

void PostWidget::onRemoveImageClicked()
{
    int currentRow = m_imagesList->currentRow();
    if (currentRow >= 0 && currentRow < m_imagePaths.size()) {
        m_imagePaths.removeAt(currentRow);
        delete m_imagesList->takeItem(currentRow);
    }
}

void PostWidget::onPostClicked()
{
    if (!validatePost()) {
        return;
    }
    
    // Reset completion tracking for new post
    m_postCompletionHandled = false;
    m_completedPosts = 0;
    
    m_progressBar->setVisible(true);
    m_statusLabel->setVisible(true);
    m_statusLabel->setText(i18n("Posting..."));
    m_postButton->setEnabled(false);
    
    QString postText = m_postText->toPlainText();
    QStringList selectedAccounts;
    
    for (QCheckBox *checkbox : m_accountCheckboxes) {
        if (checkbox->isChecked()) {
            selectedAccounts.append(checkbox->property("accountId").toString());
        }
    }
    
    m_totalAccountsToPost = selectedAccounts.size();
    qDebug() << "Posting to" << m_totalAccountsToPost << "accounts";
    
    if (selectedAccounts.isEmpty()) {
        onPostCompleted("none", false, "No accounts selected");
        return;
    }
    
    m_accountManager->postToAccounts(postText, m_imagePaths, selectedAccounts);
}

bool PostWidget::validatePost()
{
    if (m_postText->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, i18n("Empty Post"),
                            i18n("Please enter some text for your post."));
        return false;
    }
    
    if (m_postText->toPlainText().length() > MAX_CHARACTER_COUNT) {
        QMessageBox::warning(this, i18n("Post Too Long"),
                            i18n("Your post exceeds the maximum character limit of %1.")
                            .arg(MAX_CHARACTER_COUNT));
        return false;
    }
    
    bool hasSelectedAccounts = false;
    for (QCheckBox *checkbox : m_accountCheckboxes) {
        if (checkbox->isChecked()) {
            hasSelectedAccounts = true;
            break;
        }
    }
    
    if (!hasSelectedAccounts) {
        QMessageBox::warning(this, i18n("No Accounts Selected"),
                            i18n("Please select at least one account to post to."));
        return false;
    }
    
    return true;
}

void PostWidget::onPostCompleted(const QString &service, bool success, const QString &error)
{
    qDebug() << "Post completed for service:" << service << "success:" << success << "error:" << error;
    
    m_completedPosts++;
    
    if (success) {
        m_statusLabel->setText(i18n("Posted successfully to %1! (%2/%3 complete)")
                               .arg(service).arg(m_completedPosts).arg(m_totalAccountsToPost));
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_statusLabel->setText(i18n("Failed to post to %1: %2 (%3/%4 complete)")
                               .arg(service, error).arg(m_completedPosts).arg(m_totalAccountsToPost));
        m_statusLabel->setStyleSheet("color: red;");
    }
    
    // Check if all posts are complete
    if (m_completedPosts >= m_totalAccountsToPost && !m_postCompletionHandled) {
        m_postCompletionHandled = true;
        
        // Show final status
        if (m_completedPosts == m_totalAccountsToPost) {
            m_statusLabel->setText(i18n("All posts completed! (%1/%2)")
                                   .arg(m_completedPosts).arg(m_totalAccountsToPost));
        }
        
        // Use QueuedConnection to ensure safe execution
        QTimer::singleShot(2000, this, [this]() {
            if (m_progressBar && m_postButton && m_postText) { // Check if widgets still exist
                m_progressBar->setVisible(false);
                m_postButton->setEnabled(true);
                m_postCompletionHandled = false; // Reset for next post
                
                // Clear the form for a fresh start
                clearForm();
            }
        });
    }
}

void PostWidget::clearForm()
{
    m_postText->clear();
    m_imagePaths.clear();
    m_imagesList->clear();
    m_statusLabel->setVisible(false);
    m_progressBar->setVisible(false);
    m_statusLabel->setStyleSheet(""); // Reset style
    
    // Reset completion tracking
    m_postCompletionHandled = false;
    m_totalAccountsToPost = 0;
    m_completedPosts = 0;
    
    updateCharacterCount();
    onAccountSelectionChanged();
}
