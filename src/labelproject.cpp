#include "labelproject.h"

LabelProject::LabelProject(QObject *parent) : QObject(parent)
{
    connect(this, SIGNAL(video_split_finished(QString)), this, SLOT(addImageFolder(QString)));
}

void LabelProject::assignThread(QThread *thread) {
  this->moveToThread(thread);
  connect(this, SIGNAL(finished()), thread, SLOT(quit()));
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}

bool LabelProject::loadDatabase(QString fileName)
{
    /*!
     * Load and check an existing database file. If the database is valid, returns true, otherwise
     * if an error occured returns false.
     */

    if(db.isOpen()){
        db.close();
    }

    connection_name = fileName;

    db = QSqlDatabase::addDatabase("QSQLITE", connection_name);
    db.setDatabaseName(fileName);

    QFileInfo check_file(fileName);

    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
    else if ( check_file.exists() && check_file.isFile() ){
        return checkDatabase();
    }else{
        return true;
    }

}

bool LabelProject::checkDatabase(){

    /*!
     * Check that the newly loaded database contains the correct tables/fields. Returns
     * true if the database is valid, false if not.
     */

    QSqlRecord class_record = db.record("classes");
    QSqlRecord id_record = db.record("ids");
    QSqlRecord image_record = db.record("images");
    QSqlRecord label_record = db.record("labels");
    QSqlRecord meta_record = db.record("meta");

    if(class_record.isEmpty()) return false;
    if(id_record.isEmpty()) return false;
    if(image_record.isEmpty()) return false;
    if(label_record.isEmpty()) return false;
    if(meta_record.isEmpty()) return false;

    if(!class_record.contains("class_id")) return false;
    if(!class_record.contains("class_name")) return false;

    if(!id_record.contains("class_id")) return false;
    if(!id_record.contains("id")) return false;
    if(!id_record.contains("key")) return false;
    if(!id_record.contains("value")) return false;

    if(!image_record.contains("image_id")) return false;
    if(!image_record.contains("path")) return false;

    if(!label_record.contains("box_id")) return false;
    if(!label_record.contains("image_id")) return false;
    if(!label_record.contains("class_id")) return false;
    if(!label_record.contains("id")) return false;
    if(!label_record.contains("x")) return false;
    if(!label_record.contains("y")) return false;
    if(!label_record.contains("w")) return false;
    if(!label_record.contains("h")) return false;

    if(!meta_record.contains("class_id")) return false;
    if(!meta_record.contains("attribute")) return false;
    if(!meta_record.contains("value")) return false;

    qDebug() << "Database structure looks good";

    if(loadMeta()){
        qDebug() << "Data loaded successfully";
    }else{
        qDebug() << "Data loading failed";
    }


    return true;
}
bool LabelProject::loadMeta(){
    bool res;
    QSqlQuery query(db);
    QSqlQuery query_value(db);
    std::vector<QString> value_buffer;
    // TODO (kamal): clean this up

    res = query.exec("SELECT DISTINCT class_id FROM meta");
    while (query.next()) {
        int classID = query.value(0).toInt();
        QString className = getClassName(classID);
        meta.insert(std::make_pair(className, MetaObject(className)));
    }
    for(auto &metaObject: meta){
        res &= query.prepare("SELECT DISTINCT attribute FROM meta WHERE class_id = ?");
        query.bindValue(0, getClassId(metaObject.second.className));
        res &= query.exec();
        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            while (query.next()) {
                QString attribute_name = query.value(0).toString();
                res = query_value.prepare("SELECT DISTINCT value FROM meta WHERE attribute = ?");
                query_value.bindValue(0, attribute_name);
                res &= query_value.exec();
                if(!res){
                    qDebug() << "Error: " << query_value.lastError();
                }else{
                    while(query_value.next()){
                        value_buffer.push_back(query_value.value(0).toString());
                    }
                    metaObject.second.attributes.insert(std::make_pair(attribute_name, value_buffer));
                    value_buffer.clear();
                }
            }
        }
    }
    if(res){
        emit updateMeta(meta);
    }
    return res;
}
bool LabelProject::checkDuplicateId(QString className, QString id){
    QSqlQuery query(db);
    int class_id = getClassId(className);

    query.prepare("SELECT * FROM labels WHERE class_id = (:class_id) AND id = (:id)");
    query.bindValue(":class_id", class_id);
    query.bindValue(":id", id);
    bool res = query.exec();

    if(!res)
        qDebug() << "Error: " << query.lastError();
    else
        return query.next();
    return false;

}

bool LabelProject::createDatabase(QString fileName)
{
    /*!
     * Create a new labelling database at the given absolute location (\a fileName). Returns true if the database
     * was created successfully.
     */

    bool res;

    loadDatabase(fileName);
    {
        QSqlQuery query(db);

        /*
         * The label database is a simple relational db - image locations, classes
         * and label bounding boxes.
         */

        res = query.exec("CREATE table images (image_id INTEGER PRIMARY KEY ASC, "
                   "path varchar(256))");
        res &= query.exec("CREATE TABLE classes (class_id INTEGER PRIMARY KEY ASC, class_name)");
        res &= query.exec("CREATE TABLE meta (class_id INTEGER, attribute TEXT, value TEXT)");
        res &= query.exec("CREATE table ids (class_id INTEGER, id INTEGER, key TEXT, value TEXT, UNIQUE(class_id, id, key, value))");
        res &= query.exec("CREATE table labels (box_id INTEGER PRIMARY KEY, image_id INTEGER, class_id INTEGER, id INTEGER,"
                          "x INTEGER, y INTEGER, w INTEGER, h INTEGER)");

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }
    }

    return checkDatabase();
}

void LabelProject::addFolderRecursive(QString path_filter){
    /*!
     * Search for (and add) all subfolders that match \a path_filter, which
     * can include * wildcards.
     */

    // Start at the root directory in the path.
    path_filter = QDir(path_filter).absolutePath();
    QRegExp rx(path_filter);
    rx.setPatternSyntax(QRegExp::Wildcard);

    // Find the first instance of a *
    int first_wildcard = path_filter.indexOf('*');
    auto pre_wildcard = path_filter.mid(0, first_wildcard);

    // Get the top level folder
    int last_separator = pre_wildcard.lastIndexOf(QDir::separator());
    auto top_level_folder = pre_wildcard.mid(0, last_separator);

    // Recurse into this folder and find all subdirectories
    QDirIterator it(top_level_folder, QDir::Dirs, QDirIterator::Subdirectories);
    QList<QString> subfolders;
    while (it.hasNext()) {
        auto subfolder = QDir(it.next()).canonicalPath();

        if (rx.exactMatch(subfolder)){
            subfolders.append(subfolder);
        }
    }

    subfolders.removeDuplicates();
    subfolders.sort();

    for(auto &subfolder : subfolders){
        addImageFolder(subfolder);
    }

}

bool LabelProject::getImageList(QList<QString> &images)
{
    /*!
     * Get a list of all images in the database with absolute paths, which is cleared prior to retrieval. Returns false if the database query failed.
     */
    bool res = false;
    {
        QSqlQuery query(db);
        res = query.exec("SELECT path FROM images");

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{

            images.clear();

            while (query.next()) {
                QString path = query.value(0).toString();
                // Push absolute file path
                images.push_back(QDir::cleanPath(QDir(db.databaseName()).path() + QDir::separator() + path));
            }
        }
    }

    return res;
}

bool LabelProject::getClassList(QList<QString> &classes)
{
    /*!
     * Get a list of all class names in the database and puts them in (\a classes), which is cleared prior to retrieval. Returns false if the database query failed.
     */
    bool res = false;

    {
        QSqlQuery query(db);
        res = query.exec("SELECT class_name FROM classes");

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{

            classes.clear();

            while (query.next()) {
                QString name = query.value(0).toString();
                classes.push_back(name);
            }
        }
    }

    return res;
}

bool LabelProject::classInDB(QString className){
    /*!
     * Returns true if \a className is in the database
     */
    bool res = false;

    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM classes WHERE (class_name)"
                      "= (:class_name)");

        query.bindValue(":class_name", className);
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
            return false;
        }else{
            return query.next();
        }
    }
}

bool LabelProject::imageInDB(QString fileName){
    /*!
     * Returns true if \a fileName is in the database
     */
    bool res = false;
    //IF NOT EXISTS(SELECT * FROM images where (path) = (:PATH))
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM images WHERe (path)"
                      "= (:path)");

        query.bindValue(":path", QDir(db.databaseName()).relativeFilePath(fileName));
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            return query.next();
        }
    }

    return false;
}

void LabelProject::cancelLoad(){
    /*!
     * Cancel loading a video or processing a folder
     */
    should_cancel = true;
}

void LabelProject::addVideo(QString fileName, QString outputFolder){
    cv::VideoCapture video(fileName.toStdString());

    QString base_name = QFileInfo(fileName).baseName();
    QDir dir(outputFolder);

    cv::Mat frame;
    int frame_count = 0;
    int n_frames = video.get(cv::CAP_PROP_FRAME_COUNT);

    if(!video.isOpened()){
        qDebug() << "Failed to open" << fileName;
    }

    QProgressDialog progress("...", "Abort", 0, n_frames, static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Loading video");

    while(video.read(frame)){

        if(progress.wasCanceled() || frame.empty())
            break;

        QString output_name = QString("%1_%2.jpg").arg(base_name).arg(frame_count++, 6, 10, QChar('0'));

        output_name = dir.absoluteFilePath(output_name);
        cv::imwrite(output_name.toStdString(), frame);

        qDebug() << output_name;

        progress.setValue(frame_count);
        progress.setLabelText(output_name);

    }

    video.release();

    emit video_split_finished(outputFolder);

    return;
}

bool LabelProject::addAsset(QString fileName)
{
    /*!
     * Add an asset to the database, given an absolute path to the image or video (\a fileName). Returns false if the file doesn't exist or if the query failed.
     */
    bool res = false;

    if(imageInDB(fileName)){
        qDebug() << "Image exists!";
        return true;
    }
    QFileInfo check_file(fileName);

    if (  check_file.exists() && check_file.isFile() ){
        {
            QSqlQuery query(db);
            query.prepare("INSERT INTO images (path)"
                          "VALUES (:path)");

            query.bindValue(":path", QDir(db.databaseName()).relativeFilePath(fileName));
            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }
    }

    return res;
}

int LabelProject::getImageId(QString fileName){
    /*!
     * Get the image ID for an image at an absolute path (\a fileName), returns -1 if not found
     */
    int id = -1;
    {
        QSqlQuery query(db);
        query.prepare("SELECT image_id FROM images WHERE path = ?");

        //Important - we need the relative image path again
        query.bindValue(0, QDir(db.databaseName()).relativeFilePath(fileName));
        bool res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            if(query.next()){
                id = query.value(0).toInt();
            }
        }
    }

    return id;
}

int LabelProject::getClassId(QString className){
    /*!
     * Get the class ID for an class name, (\a className), returns -1 if not found
     */
    int id = -1;
    {
        QSqlQuery query(db);
        query.prepare("SELECT class_id FROM classes WHERE class_name = ?");
        query.bindValue(0, className);
        bool res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            if(query.next()){
                id = query.value(0).toInt();
            }
        }
    }

    return id;
}

QString LabelProject::getClassName(int classID){
    /*!
     * Get the class name for a class id, (\a className), returns "" if not found
     */
    QString className = "";
    {
        QSqlQuery query(db);
        query.prepare("SELECT class_name FROM classes WHERE class_id = ?");
        query.bindValue(0, classID);
        bool res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            if(query.next()){
                className= query.value(0).toString();
            }
        }
    }

    return className;
}


bool LabelProject::getLabels(QString fileName, QList<BoundingBox> &bboxes){
    bboxes.clear();
    int image_id = getImageId(fileName);
    return getLabels(image_id, bboxes);
}

bool LabelProject::getLabels(int image_id, QList<BoundingBox> &bboxes){
    /*!
     * Get the labels for a given image  (at absolute path \a fileName) and puts them in the provided list: \a bboxes. Note \a bboxes will be emptied.
     * Returns false if the query failed.
     */
    bool res = false;

    if(image_id > 0){
        {
            QSqlQuery query(db);
            QSqlQuery query_attributes(db);

            query.prepare("SELECT class_name, id, x, y, w, h FROM labels"
                          " INNER JOIN classes ON labels.class_id = classes.class_id"
                          " WHERE image_id = ?");
            query.addBindValue(image_id);
            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }

            while(query.next()){
                BoundingBox new_bbox;
                auto rec = query.record();

                new_bbox.classname = rec.value(rec.indexOf("class_name")).toString();
                new_bbox.classid = getClassId(new_bbox.classname);
                new_bbox.id = rec.value(rec.indexOf("id")).toInt();

                query_attributes.prepare("SELECT key, value FROM ids WHERE id = (:id)"
                                         " AND class_id = (:class_id)");
                query_attributes.bindValue(":id", new_bbox.id);
                query_attributes.bindValue(":class_id", new_bbox.classid);
                query_attributes.exec();

                while(query_attributes.next()){
                    auto rec_attributes = query_attributes.record();
                    QString attribute = rec_attributes.value(rec_attributes.indexOf("key")).toString();
                    QString value = rec_attributes.value(rec_attributes.indexOf("value")).toString();
                    new_bbox.attributes.insert(std::make_pair(attribute, value));
                }

                new_bbox.rect.setX(rec.value(rec.indexOf("x")).toInt());
                new_bbox.rect.setY(rec.value(rec.indexOf("y")).toInt());
                new_bbox.rect.setWidth(rec.value(rec.indexOf("w")).toInt());
                new_bbox.rect.setHeight(rec.value(rec.indexOf("h")).toInt());

                bboxes.append(new_bbox);
            }
        }
    }

    return res;
}

bool LabelProject::removeLabels(QString fileName){
    /*!
     * Remove all labels from an image given an absolute path to the image (\a fileName).
     */
    int image_id = getImageId(fileName);

    bool res = false;

    if(image_id > 0){
        {
            QSqlQuery query(db);
            /*
            query.prepare("SELECT id FROM labels WHERE (image_id = :image_id");
            query.bindValue(":image_id", image_id);
            query.exec();

            while (query.next()) {
                int classID = query.value(0).toInt();
                query.prepare("DELETE FROM ids WHERE (id = :id)");
            }
            */

            query.prepare("DELETE FROM labels WHERE (image_id = :image_id)");
            query.bindValue(":image_id", image_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }

    }

    return res;
}

bool LabelProject::removeLabel(QString fileName, BoundingBox bbox){
    /*!
     * Remove a label given an absolute path  (\a fileName) and the bounding box  (\a bbox). Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    bool res = false;

    if(image_id > 0 && class_id > 0 && bbox.id > 0){
        {
            QSqlQuery query(db);

            query.prepare("DELETE FROM labels WHERE (image_id = :image_id AND class_id = :class_id"
                          " AND id = :id AND x = :x AND y = :y AND w = :w AND h = :h)");
            query.bindValue(":image_id", image_id);
            query.bindValue(":class_id", class_id);
            query.bindValue(":id", bbox.id);
            query.bindValue(":x", bbox.rect.x());
            query.bindValue(":y", bbox.rect.y());
            query.bindValue(":w", bbox.rect.width());
            query.bindValue(":h", bbox.rect.height());

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }

            query.prepare("DELETE FROM ids WHERE class_id = :class_id AND id = :id");
            query.bindValue(":class_id", class_id);
            query.bindValue(":id", bbox.id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }
    }

    return res;
}

bool LabelProject::setOccluded(QString fileName, BoundingBox bbox, int occluded){
    /*!
     * Set a label to be occluded. Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    bool res = false;

    if(image_id > 0 && class_id > 0){
        {
            QSqlQuery query(db);

            query.prepare("UPDATE labels SET occluded = :occluded WHERE (image_id = :image_id AND class_id = :class_id"
                          " AND x = :x AND y = :y AND width = :width AND height = :height)");
            query.bindValue(":image_id", image_id);
            query.bindValue(":class_id", class_id);
            query.bindValue(":x", bbox.rect.x());
            query.bindValue(":y", bbox.rect.y());
            query.bindValue(":width", bbox.rect.width());
            query.bindValue(":height", bbox.rect.height());
            query.bindValue(":occluded", occluded);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }
    }

    return res;
}
bool LabelProject::removeClass(QString className)
{
    int class_id = getClassId(className);

    bool res = false;
    if (class_id > 0)
    {
            QSqlQuery query(db);
            query.prepare("DELETE FROM classes WHERE (class_id = :class_id)");
            query.bindValue(":class_id", class_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }else{
                meta.erase(className);
                emit updateMeta(meta);
            }
    }
    return res;
}

bool LabelProject::removeClassLabels(QString className){
    /*!
     * Remove a class given a class name (\a className). Also removes all labels with that class. Returns false
     * if the query failed.
     */
    int class_id = getClassId(className);

    bool res = false;

    if(class_id > 0){
        {
            QSqlQuery query(db);

            // Delete all labels with this class
            query.prepare("DELETE FROM labels WHERE (class_id = :class_id)");
            query.bindValue(":class_id", class_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }

            // Delete the class itself
            query.prepare("DELETE FROM classes WHERE (class_id = :class_id)");
            query.bindValue(":class_id", class_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }
    }

    return res;
}

int LabelProject::getNextUnlabelled(QString fileName){
    /*!
     * Returns the index of the next unlabelled image, after \a fileName
     */
    QList<QString> images;
    getImageList(images);

    QList<BoundingBox> bboxes;

    for(int i = images.indexOf(fileName) + 1; i < images.size(); i++){
        bboxes.clear();
        getLabels(images.at(i), bboxes);

        if(bboxes.size() == 0) return i+1;
    }

    return -1;
}

int LabelProject::getNextInstance(QString fileName, QString className){
    /*!
     * Returns the index of the next unlabelled image, after \a fileName
     */
    QList<QString> images;
    getImageList(images);

    int i = images.indexOf(fileName);
    QList<BoundingBox> bboxes;

    for(i += 1; i < images.size(); i++){
        bboxes.clear();
        getLabels(images.at(i), bboxes);

        for(auto bbox : bboxes){
            if(bbox.classname.compare(className) == 0){
                return i;
            }
        }
    }

    return -1;
}

bool LabelProject::removeImage(QString fileName){

    /*!
     * Remove an image given an absolute path (\a fileName). Also removes all labels for that image. Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);

    bool res = false;

    if(image_id > 0){
        {
            QSqlQuery query(db);

            // Delete all labels for this image
            query.prepare("DELETE FROM labels WHERE (image_id = :image_id)");
            query.bindValue(":image_id", image_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }

            // Delete the image itself
            query.prepare("DELETE FROM images WHERE (image_id = :image_id)");
            query.bindValue(":image_id", image_id);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }

    }

    return res;
}

bool LabelProject::addLabel(QString fileName, BoundingBox bbox)
{
    /*!
     * Add a label given an absolute path (\a fileName) and the bounding box (\a bbox). Returns false
     * if the query failed.
     */

    QMutexLocker locker(&mutex);

    bool res = false;

    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    if(image_id > 0 && class_id > 0 && bbox.id > 0){
        {
            QSqlQuery query(db);

            for (auto &attribute : bbox.attributes){
                query.prepare("INSERT INTO ids (class_id, id, key, value) VALUES (:class_id, :id, :key, :value)");
                query.bindValue(":class_id", class_id);
                query.bindValue(":id", bbox.id);
                query.bindValue(":key", attribute.first);
                query.bindValue(":value", attribute.second);
                res &= query.exec();
                if (!res)
                    qDebug() << "Error: " << query.lastError();
            }

            query.prepare("INSERT INTO labels (image_id, class_id, id, x, y, w, h)"
                          "VALUES (:image_id, :class_id, :id, :x, :y, :w, :h)");
            query.bindValue(":image_id", image_id);
            query.bindValue(":class_id", class_id);
            query.bindValue(":id", bbox.id);
            query.bindValue(":x", bbox.rect.x());
            query.bindValue(":y", bbox.rect.y());
            query.bindValue(":w", bbox.rect.width());
            query.bindValue(":h", bbox.rect.height());
            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }
        }
    }

    return res;
}


int LabelProject::addImageFolder(QString path){
    /*!
     * Add all images in a folder given an absolute \a path to the directory. Returns the number of images successfully added.
     */
    QDir dir(path);
    int number_added = 0;

    if(dir.exists()){
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff";
        auto image_list = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

        QFileInfo image_info;

        QSqlDatabase::database().transaction();

        QProgressDialog progress("...", "Abort", 0, image_list.size(), static_cast<QWidget*>(parent()));
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle("Loading images");
        int i=0;

        foreach(image_info, image_list){

            if(progress.wasCanceled()){
                break;
            }

            QString image_path = image_info.absoluteFilePath();

            bool res = addAsset(image_path);

            if(res){
                number_added++;
                qDebug() << "Added image: " << image_path;
            }else{
                qDebug() << "Failed to add image: " << image_path;
            }

            progress.setValue(i++);
            progress.setLabelText(image_path);
            QApplication::processEvents();

        }
        QSqlDatabase::database().commit();
    }

    emit load_finished();

    return number_added;

}

bool LabelProject::addClass(QString className)
{
    /*!
     * Add a class with name \a className. Returns false
     * if the query failed.
     */
    bool res = false;

    if(classInDB(className)){
        qDebug() << "Class exists!";
        return true;
    }

    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO classes (class_name)"
                      "VALUES (:class_name)");
        query.bindValue(":class_name", className);
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }else{
            meta.insert(std::make_pair(className, MetaObject(className)));
            emit updateMeta(meta);
        }
    }

    return res;
}

LabelProject::~LabelProject()
{
    /*!
     * Destructor, closes the database if it's open.
     */

    if(db.isOpen()){
        qDebug() << "Closing database.";
        db.close();
        QSqlDatabase::removeDatabase(connection_name);
    }

    emit finished();
}

bool LabelProject::addValue(QString newValue, QString currentAttribute, QString currentClass)
{
    int classID = getClassId(currentClass);

    bool res = false;

    if(classID > 0){
        {
            QSqlQuery query(db);

            query.prepare("INSERT INTO meta (class_id, attribute, value)"
                          "VALUES (:class_id, :attribute, :value)");
            query.bindValue(":class_id", classID);
            query.bindValue(":attribute", currentAttribute);
            query.bindValue(":value", newValue);
            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }else{
                meta[currentClass].attributes[currentAttribute].push_back(newValue);
                emit updateMeta(meta);
            }
        }
    }

    return res;
}

bool LabelProject::deleteValue(QString value, QString currentAttribute, QString currentClass)
{
    bool res = false;
    int classID = getClassId(currentClass);
    if(classID > 0){
        {
            QSqlQuery query(db);

            query.prepare("DELETE FROM meta WHERE (class_id = :class_id AND attribute = :attribute"
                          " AND value = :value)");
            query.bindValue(":class_id", classID);
            query.bindValue(":attribute", currentAttribute);
            query.bindValue(":value", value);

            res = query.exec();

            if(!res){
                qDebug() << "Error: " << query.lastError();
            }else{
                // TODO (kamal): use unordered set to remove duplicate values and clean
                std::vector<QString> *temp = &meta[currentClass].attributes[currentAttribute];
                temp->erase(std::remove(temp->begin(), temp->end(), value), temp->end());
                emit updateMeta(meta);
            }
        }
    }
    return res;
}

void LabelProject::sendMeta(){
   emit updateMeta(meta);
}
