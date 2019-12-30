#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menuSettings->setEnabled(false);
    ui->actionimport_labels->setEnabled(false);

    ui->removeLabelsForwardButton->setEnabled(false);

    // main toolbar "File" connections
    connect(ui->actionNew_Project, SIGNAL(triggered(bool)), this, SLOT(newProject()));
    connect(ui->actionOpen_Project, SIGNAL(triggered(bool)), this, SLOT(openProject()));
    connect(ui->actionMerge_Project, SIGNAL(triggered(bool)), this, SLOT(mergeProject()));

    // main toolbar "Images" connections
    connect(ui->actionAdd_video, SIGNAL(triggered(bool)), this, SLOT(addVideo()));
    connect(ui->actionAdd_image, SIGNAL(triggered(bool)), this, SLOT(addImages()));
    connect(ui->actionAdd_image_folder, SIGNAL(triggered(bool)), this, SLOT(addImageFolder()));
    connect(ui->actionAdd_image_folders, SIGNAL(triggered(bool)), this, SLOT(addImageFolders()));

    // Navigation connections
    connect(ui->actionNextImage, SIGNAL(triggered(bool)), this, SLOT(nextImage()));
    connect(ui->actionPreviousImage, SIGNAL(triggered(bool)), this, SLOT(previousImage()));
    connect(ui->actionJump_forward, SIGNAL(triggered(bool)), this, SLOT(jumpForward()));
    connect(ui->actionJump_backward, SIGNAL(triggered(bool)), this, SLOT(jumpBackward()));

    // Importer connection
    connect(ui->actionimport_labels, SIGNAL(triggered(bool)), this, SLOT(importLabels()));

    // Class and Attribute Dialog connections
    connect(ui->actionAdd_Remove_Class, SIGNAL(triggered(bool)), this, SLOT(addRemoveClass()));
    connect(ui->actionAdd_Remove_Attributes, SIGNAL(triggered(bool)), this, SLOT(addRemoveAttributes()));
    connect(classDialog, SIGNAL(addClass(QString)), this, SLOT(addClass(QString)));
    connect(this, SIGNAL(updateClassList(QList<QString>)), classDialog, SLOT(getClassList(QList<QString>)));
    connect(classDialog, SIGNAL(deleteClass(QString)), this, SLOT(removeClass(QString)));
    connect(this, SIGNAL(updateClassList(QList<QString>)), attrDialog, SLOT(getClassList(QList<QString>)));

    // Tracking connections
    connect(ui->actionInit_Tracking, SIGNAL(triggered(bool)), this, SLOT(initTrackers()));
    connect(ui->actionPropagate_Tracking, SIGNAL(triggered(bool)), this, SLOT(updateTrackers()));
    connect(ui->propagateCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggleAutoPropagate(bool)));

    // Next unlabelled and Next Instance connections
    connect(ui->nextUnlabelledButton, SIGNAL(clicked(bool)), this, SLOT(nextUnlabelled()));
    connect(ui->nextInstanceButton, SIGNAL(clicked(bool)), this, SLOT(nextInstance()));

    display = new ImageDisplay;
    ui->imageDisplayLayout->addWidget(display);
    currentImage = display->getImageLabel();

    // Image connections
    connect(this, SIGNAL(updateClassList(QList<QString>)), currentImage->inputDialog, SLOT(getClassList(QList<QString>)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));
    connect(currentImage, SIGNAL(updateLabel(BoundingBox, BoundingBox)), this, SLOT(updateLabel(BoundingBox, BoundingBox)));
    connect(currentImage, SIGNAL(selectLabel(BoundingBox)), this, SLOT(updateLabelInfo(BoundingBox)));
    connect(currentImage, SIGNAL(deselectLabel()), this, SLOT(defaultLabelInfo()));

    connect(display, SIGNAL(image_loaded()), this, SLOT(updateImageInfo()));

    // Remove Image, Remove Labels and Remove Labels forward connections
    connect(ui->removeImageButton, SIGNAL(clicked(bool)), this, SLOT(removeImage()));
    connect(ui->removeImageLabelsButton, SIGNAL(clicked(bool)), this, SLOT(removeImageLabels()));
    connect(ui->removeLabelsForwardButton, SIGNAL(clicked(bool)), this, SLOT(removeImageLabelsForward()));

    // Jump to image connections
    connect(ui->changeImageButton, SIGNAL(clicked(bool)), this, SLOT(updateDisplay()));
    connect(ui->imageNumberSpinbox, SIGNAL(editingFinished()), this, SLOT(updateDisplay()));

    // Wrap images, Export connections
    connect(ui->actionWrap_images, SIGNAL(triggered(bool)), this, SLOT(enableWrap(bool)));
    connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(launchExportDialog()));

    //connect(ui->actionSetup_detector, SIGNAL(triggered(bool)), this, SLOT(setupDetector()));

    auto prev_shortcut = ui->actionPreviousImage->shortcuts();
    prev_shortcut.append(QKeySequence("Left"));
    ui->actionPreviousImage->setShortcuts(prev_shortcut);

    auto next_shortcut = ui->actionNextImage->shortcuts();
    next_shortcut.append(QKeySequence("Right"));
    ui->actionNextImage->setShortcuts(next_shortcut);

    // Override progress bar animation on Windows
#ifdef WIN32
    ui->imageProgressBar->setStyleSheet("QProgressBar::chunk {background-color: #3add36; width: 1px;}");
#endif

    settings = new QSettings("DeepLabel", "DeepLabel");

    multitracker = new MultiTrackerCV();
    reinterpret_cast<MultiTrackerCV *>(multitracker)->setTrackerType(CSRT);

    // DarkStyle qss load
    QFile f(":qdarkstyle/style.qss");
    if(!f.exists())
        printf("Unable to set stylesheet, file not found \n");
    else{
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }


    // QtAwesome font load
    QtAwesome* awesome = new QtAwesome(qApp);
    awesome->initFontAwesome();

    QVariantMap options;
    options.insert( "color" , QColor(30,30,30) );
    options.insert( "scale-factor", 0.7 );

    ui->actionPreviousImage->setIcon(awesome->icon(fa::arrowleft, options));
    ui->actionNextImage->setIcon(awesome->icon(fa::arrowright, options));

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

}

void MainWindow::mergeProject(QString filename){

    if(filename == ""){
        QString openDir = settings->value("project_folder", QDir::homePath()).toString();
        filename = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                        openDir,
                                                        tr("Label database (*.lbldb)"));
    }

    if(filename == "") return;

    LabelProject new_project;
    new_project.loadDatabase(filename);

    // Add new classes
    QList<QString> new_classes;
    new_project.getClassList(new_classes);

    qDebug() << "Found " << new_classes.size() << " classes.";

    for(auto &classname : new_classes){
        project->addClass(classname);
    }

    // Add new images
    QList<QString> new_images;
    new_project.getImageList(new_images);

    qDebug() << "Found " << new_images.size() << " images.";

    for(auto &image : new_images){
        // Add image
        auto res = project->addAsset(image);

        if(!res){
            qDebug() << "Problem adding: " << image;
        }else{
            qDebug() << "Added: " << image;
        }

        // Add labels for image
        QList<BoundingBox> bboxes;
        new_project.getLabels(image, bboxes);

        for(auto &bbox : bboxes){
            // Update the class ID
            bbox.classid = project->getClassId(bbox.classname);
            project->addLabel(image, bbox);
        }
    }

    updateImageList();
    updateClassList();
    updateDisplay();
}

void MainWindow::updateLabelInfo(BoundingBox bbox){
    // Label Information panel
    if(ui->labelClass->text() == "-"){
        ui->labelClass->setText(bbox.classname);
        ui->labelID->setText(QString::number(bbox.id));
        int attributeIndex = 0;
        for(auto &attribute : bbox.attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attributeIndex, 0);
            ui->gridLayout->addWidget(new QLabel(attribute.second), attributeIndex, 1);
            attributeIndex ++;
        }
    }
}

void clearGridLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearGridLayout(item->layout());
        delete item->layout();
        }
        if (item->widget()) {
           delete item->widget();
        }
        delete item;
    }
}

void MainWindow::defaultLabelInfo(){
    // default Label Information view
    ui->labelClass->setText("-");
    ui->labelID->setText("-");
    clearGridLayout(ui->gridLayout);
}

void MainWindow::addRemoveClass(){
    // launch Class configuration dialogue
    updateClassList();
    classDialog->load();
    classDialog->exec();
}

void MainWindow::addRemoveAttributes(){
    // launch Attribute configuration dialogue
    attrDialog->load();
    attrDialog->exec();
}

void MainWindow::toggleAutoPropagate(bool state){
    track_previous = state;
}

void MainWindow::enableWrap(bool enable){
    wrap_index = enable;
}

void MainWindow::jumpForward(int n){

    if(ui->imageNumberSpinbox->maximum() == 0) return;

    current_index = std::min(ui->imageNumberSpinbox->maximum()-1, ui->imageNumberSpinbox->value()+n);
    ui->imageNumberSpinbox->setValue(current_index);
    updateDisplay();
}

void MainWindow::jumpBackward(int n){

    current_index = std::max(1, ui->imageNumberSpinbox->value()-n);
    ui->imageNumberSpinbox->setValue(current_index);
    updateDisplay();
}

void MainWindow::openProject(QString fileName)
{

    if(fileName == ""){
        QString openDir = settings->value("project_folder", QDir::homePath()).toString();
        fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                        openDir,
                                                        tr("Label database (*.lbldb)"));
    }

    if(fileName != ""){
        settings->setValue("project_folder", QFileInfo(fileName).absoluteDir().absolutePath());
        projectName = fileName;
        if(project->loadDatabase(fileName)){
            initDisplay();
            setWindowTitle("DeepLabel - " + fileName);
            ui->menuSettings->setEnabled(true);
        }else{
            QMessageBox::warning(this,tr("Remove Image"), tr("Failed to open project."));
            setWindowTitle("DeepLabel");
        }
    }
    if(project != nullptr){
        // Dialog connections to database interface
        connect(attrDialog, SIGNAL(addValue(QString, QString, QString)), project, SLOT(addValue(QString, QString, QString)));
        connect(attrDialog, SIGNAL(deleteValue(QString, QString, QString)), project, SLOT(deleteValue(QString, QString, QString)));
        connect(currentImage->inputDialog, SIGNAL(getMeta()), project, SLOT(sendMeta()));
        connect(currentImage->inputDialog, SIGNAL(checkDuplicateId(QString, QString)), project, SLOT(checkDuplicateId(QString, QString)));
        connect(currentImage->inputDialog, SIGNAL(getMaxID(QString)), project, SLOT(sendMaxID(QString)));
        connect(project, SIGNAL(updateMeta(std::map<QString, MetaObject>)), currentImage->inputDialog, SLOT(updateMeta(std::map<QString, MetaObject>)));
        connect(project, SIGNAL(updateMeta(std::map<QString, MetaObject>)), attrDialog, SLOT(updateMeta(std::map<QString, MetaObject>)));
    }

    return;
}

void MainWindow::updateLabels(){
    QList<BoundingBox> bboxes;
    project->getLabels(current_imagepath, bboxes);

    ui->instanceCountLabel->setNum(bboxes.size());
    currentImage->setBoundingBoxes(bboxes);
}

void MainWindow::addImageFolders(void){

    QDialog  image_folder_dialog(this);

    auto path_edit = new QLineEdit();
    auto ok_button = new QPushButton("Ok");
    auto path_label = new QLabel("Folder path (wildcards allowed)");

    image_folder_dialog.setWindowTitle("Add folders");
    image_folder_dialog.setLayout(new QVBoxLayout());
    image_folder_dialog.layout()->addWidget(path_label);
    image_folder_dialog.layout()->addWidget(path_edit);
    image_folder_dialog.layout()->addWidget(ok_button);

    connect(ok_button, SIGNAL(clicked(bool)), &image_folder_dialog, SLOT(accept()));

    image_folder_dialog.exec();

    if(image_folder_dialog.result() == QDialog::Accepted){
        project->addFolderRecursive(path_edit->text());
    }
}

void MainWindow::updateImageList(){
    project->getImageList(images);
    number_images = images.size();

    if(number_images == 0){
        ui->imageGroupBox->setDisabled(true);
        ui->labelGroupBox->setDisabled(true);
        ui->navigationGroupBox->setDisabled(true);
        ui->actionExport->setDisabled(true);
        ui->imageIndexLabel->setText(QString("-"));
    }else{
        ui->imageGroupBox->setEnabled(true);
        ui->labelGroupBox->setEnabled(true);
        ui->navigationGroupBox->setEnabled(true);
        ui->actionExport->setEnabled(true);
        ui->imageProgressBar->setValue(0);
        ui->imageProgressBar->setMaximum(number_images);
        ui->actionimport_labels->setEnabled(true);
    }

    ui->imageNumberSpinbox->setMaximum(number_images);
    ui->imageNumberSpinbox->setValue(1);

}

void MainWindow::updateClassList(){
    project->getClassList(classes);
    emit updateClassList(classes);
}

void MainWindow::addClass(QString new_class){

    if(new_class.simplified() != "" && !classes.contains(new_class)){
        project->addClass(new_class.simplified());
        updateClassList();
    }
}

void MainWindow::addLabel(BoundingBox bbox){
    project->addLabel(current_imagepath, bbox);
    updateLabels();
}

void MainWindow::removeLabel(BoundingBox bbox){
    project->removeLabel(current_imagepath, bbox);
    updateLabels();
}

void MainWindow::removeImageLabels(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Labels"),
                                                  QString("Really delete all labels for this image?"))){;
        project->removeLabels(current_imagepath);
        updateLabels();
        updateDisplay();
    }
}

void MainWindow::removeImageLabelsForward(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Labels"),
                                                  QString("Really delete all labels forwards?"))){;

        while(true){

            QList<BoundingBox> bboxes;
            project->getLabels(current_imagepath, bboxes);

            if(bboxes.size() == 0){
                return;
            }

            if(current_index == (number_images-1)){
                    return;
            }else{
                current_index++;
            }

            ui->imageNumberSpinbox->setValue(current_index+1);
            project->removeLabels(current_imagepath);

            // Show the new image
            updateLabels();
            updateDisplay();
        }

    }
}

void MainWindow::updateLabel(BoundingBox old_bbox, BoundingBox new_bbox){

    project->removeLabel(current_imagepath, old_bbox);
    project->addLabel(current_imagepath, new_bbox);
    updateLabels();
}

void MainWindow::removeImage(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Image"),
                                                  tr("Really delete image and associated labels?"))){
        project->removeImage(current_imagepath);
        updateImageList();
        updateDisplay();
    }
}
void MainWindow::removeClass(QString class_name){
    project->removeClass(class_name);
    updateClassList();
}

void MainWindow::removeClass(){

    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Class"),
                                                  QString("Really delete all \"%1\" labels from your entire dataset?")
                                                    .arg(current_class))){;
        project->removeClass(current_class);
        updateClassList();
        updateDisplay();
    }
}

void MainWindow::initDisplay(){

    display->clearPixmap();

    updateImageList();
    updateClassList();

    current_index = 0;

    updateDisplay();
}

void MainWindow::nextUnlabelled(){
    int n = project->getNextUnlabelled(current_imagepath);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n);
        updateDisplay();
    }
}

void MainWindow::nextInstance(void){
    int n = project->getNextInstance(current_imagepath, current_class);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n+1);
        updateDisplay();
    }
}

void MainWindow::initTrackers(void){
    multitracker->init(currentImage->getImage(), currentImage->getBoundingBoxes());
}

void MainWindow::updateTrackers(void){

    // If there are no labels, and we're tracking the previous frame
    // propagate the bounding boxes. Otherwise we assume that the
    // current labels are the correct ones and should override.

    auto image = currentImage->getImage();
    if(image.empty()) return;

    multitracker->update(image);

    auto new_bboxes = multitracker->getBoxes();

    for(auto &new_bbox : new_bboxes){
        project->addLabel(current_imagepath, new_bbox);
    }

    updateLabels();
}

void MainWindow::nextImage(){

    if(images.empty()) return;

    if(current_index == (number_images-1)){
        if(wrap_index){
            current_index = 0;
        }else{
            return;
        }
    }else{
        current_index++;
    }

    ui->imageNumberSpinbox->setValue(current_index+1);

    if(track_previous)
        initTrackers();
    // Show the new image
    updateDisplay();

    // Only auto-propagate if we've enabled it and there are no boxes in the image already.
    if(track_previous && currentImage->getBoundingBoxes().size() == 0){
        updateTrackers();
    }

    updateLabels();
}

void MainWindow::previousImage(){

    if(images.empty()) return;

    if(current_index == 0){
        if(wrap_index){
            current_index = number_images - 1;
        }else{
            return;
        }
    }else{
      current_index--;
    }

    ui->imageNumberSpinbox->setValue(current_index+1);
    updateDisplay();
}

void MainWindow::updateCurrentIndex(int index){
    current_index = index;
    updateDisplay();
}

void MainWindow::updateDisplay(){

    if(images.size() == 0){
        return;
    }else{
        current_index = ui->imageNumberSpinbox->value()-1;
        current_imagepath = images.at(current_index);
        display->setImagePath(current_imagepath);

        updateLabels();

        ui->imageProgressBar->setValue(current_index+1);
        ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index+1).arg(number_images));
    }
}

void MainWindow::updateImageInfo(void){
    auto image_info = QFileInfo(current_imagepath);
    ui->imageBitDepthLabel->setText(QString("%1 bit").arg(display->getBitDepth()));
    ui->filenameLabel->setText(image_info.fileName());
    ui->filenameLabel->setToolTip(image_info.fileName());
    ui->filetypeLabel->setText(image_info.completeSuffix());
    ui->sizeLabel->setText(QString("%1 kB").arg(image_info.size() / 1000));
    ui->dimensionsLabel->setText(QString("(%1, %2) px").arg(currentImage->getImage().cols).arg(currentImage->getImage().rows));
}

void MainWindow::newProject()
{
    QString openDir = settings->value("project_folder", QDir::homePath()).toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        free(project);
        project = new LabelProject;
        project->createDatabase(fileName);
        openProject(fileName);
    }

    return;
}

void MainWindow::addVideo(void){
    QString openDir = QDir::homePath();
    QString video_filename = QFileDialog::getOpenFileName(this, tr("Select video"),
                                                    openDir);

    QString output_folder = QFileDialog::getExistingDirectory(this, "Output folder", openDir);

    if(video_filename != ""){
        project->addVideo(video_filename, output_folder);
    }

    updateImageList();
    initDisplay();

}

void MainWindow::addImages(void){
    QString openDir = settings->value("data_folder", QDir::homePath()).toString();
    QStringList image_filenames = QFileDialog::getOpenFileNames(this, tr("Select image(s)"),
                                                    openDir,
                                                    tr("JPEG (*.jpg *.jpeg *.JPG *.JPEG);;PNG (*.png *.PNG);;BMP (*.bmp *.BMP);;TIFF (*.tif *.tiff *.TIF *.TIFF);;All images (*.jpg *.jpeg *.png *.bmp *.tiff)"));

    if(image_filenames.size() != 0){
        QString path;

        QProgressDialog progress("Loading images", "Abort", 0, image_filenames.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        int i=0;

        foreach(path, image_filenames){
            if(progress.wasCanceled()){
                break;
            }

            project->addAsset(path);
            progress.setValue(i++);
        }

        settings->setValue("data_folder", QDir(image_filenames.at(0)).dirName());
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::addImageFolder(void){
    QString openDir = settings->value("data_folder", QDir::homePath()).toString();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select image folder"),
                                                    openDir);

    if(path != ""){
        int number_added = project->addImageFolder(path);
        settings->setValue("data_folder", path);
        qDebug() << "Added: " << number_added << " images";
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::handleExportDialog(){

    // If we hit OK and not cancel
    if(export_dialog->result() != QDialog::Accepted ) return;

    QThread* export_thread = new QThread;

    Exporter exporter(project);
    exporter.moveToThread(export_thread);
    exporter.setOutputFolder(export_dialog->getOutputFolder());
    exporter.setDataName(projectName);
    exporter.setContributer(export_dialog->getContributer());
    exporter.export_labels();
}

void MainWindow::launchExportDialog(){

    export_dialog = new ExportDialog(this);

    export_dialog->setModal(true);
    connect(export_dialog, SIGNAL(accepted()), this, SLOT(handleExportDialog()));

    export_dialog->open();
}

void MainWindow::importLabels(){

    // get label .json file
    QString openDir = settings->value("project_folder", QDir::homePath()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Labels"),
                                                        openDir,
                                                        tr("Label file (*.json)"));

    Importer importer(project);
    importer.load(fileName);
    updateClassList();
    updateLabels();
}

MainWindow::~MainWindow()
{
    delete ui;
}
