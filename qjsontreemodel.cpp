#include "qjsontreemodel.h"

namespace QUtf8Functions
{
    /// returns 0 on success; errors can only happen if \a u is a surrogate:
    /// Error if \a u is a low surrogate;
    /// if \a u is a high surrogate, Error if the next isn't a low one,
    /// EndOfString if we run into the end of the string.
    template <typename Traits, typename OutputPtr, typename InputPtr> inline
    int toUtf8(ushort u, OutputPtr &dst, InputPtr &src, InputPtr end)
    {
        if (!Traits::skipAsciiHandling && u < 0x80)
        {
            // U+0000 to U+007F (US-ASCII) - one byte
            Traits::appendByte(dst, uchar(u));
            return 0;
        }
        else if (u < 0x0800)
        {
            // U+0080 to U+07FF - two bytes
            // first of two bytes
            Traits::appendByte(dst, 0xc0 | uchar(u >> 6));
        }
        else
        {
            if (!QChar::isSurrogate(u))
            {
                // U+0800 to U+FFFF (except U+D800-U+DFFF) - three bytes
                if (!Traits::allowNonCharacters && QChar::isNonCharacter(u))
                    return Traits::Error;
                // first of three bytes
                Traits::appendByte(dst, 0xe0 | uchar(u >> 12));
            }
            else
            {
                // U+10000 to U+10FFFF - four bytes
                // need to get one extra codepoint
                if (Traits::availableUtf16(src, end) == 0)
                    return Traits::EndOfString;
                ushort low = Traits::peekUtf16(src);
                if (!QChar::isHighSurrogate(u))
                    return Traits::Error;
                if (!QChar::isLowSurrogate(low))
                    return Traits::Error;
                Traits::advanceUtf16(src);
                uint ucs4 = QChar::surrogateToUcs4(u, low);
                if (!Traits::allowNonCharacters && QChar::isNonCharacter(ucs4))
                    return Traits::Error;
                // first byte
                Traits::appendByte(dst, 0xf0 | (uchar(ucs4 >> 18) & 0xf));
                // second of four bytes
                Traits::appendByte(dst, 0x80 | (uchar(ucs4 >> 12) & 0x3f));
                // for the rest of the bytes
                u = ushort(ucs4);
            }
            // second to last byte
            Traits::appendByte(dst, 0x80 | (uchar(u >> 6) & 0x3f));
        }
        // last byte
        Traits::appendByte(dst, 0x80 | (u & 0x3f));
        return 0;
    }
    inline bool isContinuationByte(uchar b)
    {
        return (b & 0xc0) == 0x80;
    }
    /// returns the number of characters consumed (including \a b) in case of success;
    /// returns negative in case of error: Traits::Error or Traits::EndOfString
    template <typename Traits, typename OutputPtr, typename InputPtr> inline
    int fromUtf8(uchar b, OutputPtr &dst, InputPtr &src, InputPtr end)
    {
        int charsNeeded;
        uint min_uc;
        uint uc;
        if (!Traits::skipAsciiHandling && b < 0x80)
        {
            // US-ASCII
            Traits::appendUtf16(dst, b);
            return 1;
        }
        if (!Traits::isTrusted && Q_UNLIKELY(b <= 0xC1))
        {
            // an UTF-8 first character must be at least 0xC0
            // however, all 0xC0 and 0xC1 first bytes can only produce overlong sequences
            return Traits::Error;
        }
        else if (b < 0xe0)
        {
            charsNeeded = 2;
            min_uc = 0x80;
            uc = b & 0x1f;
        }
        else if (b < 0xf0)
        {
            charsNeeded = 3;
            min_uc = 0x800;
            uc = b & 0x0f;
        }
        else if (b < 0xf5)
        {
            charsNeeded = 4;
            min_uc = 0x10000;
            uc = b & 0x07;
        }
        else
        {
            // the last Unicode character is U+10FFFF
            // it's encoded in UTF-8 as "\xF4\x8F\xBF\xBF"
            // therefore, a byte higher than 0xF4 is not the UTF-8 first byte
            return Traits::Error;
        }
        int bytesAvailable = Traits::availableBytes(src, end);
        if (Q_UNLIKELY(bytesAvailable < charsNeeded - 1))
        {
            // it's possible that we have an error instead of just unfinished bytes
            if (bytesAvailable > 0 && !isContinuationByte(Traits::peekByte(src, 0)))
                return Traits::Error;
            if (bytesAvailable > 1 && !isContinuationByte(Traits::peekByte(src, 1)))
                return Traits::Error;
            return Traits::EndOfString;
        }
        // first continuation character
        b = Traits::peekByte(src, 0);
        if (!isContinuationByte(b))
            return Traits::Error;
        uc <<= 6;
        uc |= b & 0x3f;
        if (charsNeeded > 2)
        {
            // second continuation character
            b = Traits::peekByte(src, 1);
            if (!isContinuationByte(b))
                return Traits::Error;
            uc <<= 6;
            uc |= b & 0x3f;
            if (charsNeeded > 3)
            {
                // third continuation character
                b = Traits::peekByte(src, 2);
                if (!isContinuationByte(b))
                    return Traits::Error;
                uc <<= 6;
                uc |= b & 0x3f;
            }
        }
        // we've decoded something; safety-check it
        if (!Traits::isTrusted)
        {
            if (uc < min_uc)
                return Traits::Error;
            if (QChar::isSurrogate(uc) || uc > QChar::LastValidCodePoint)
                return Traits::Error;
            if (!Traits::allowNonCharacters && QChar::isNonCharacter(uc))
                return Traits::Error;
        }
        // write the UTF-16 sequence
        if (!QChar::requiresSurrogates(uc))
        {
            // UTF-8 decoded and no surrogates are required
            // detach if necessary
            Traits::appendUtf16(dst, ushort(uc));
        }
        else
        {
            // UTF-8 decoded to something that requires a surrogate pair
            Traits::appendUcs4(dst, uc);
        }
        Traits::advanceByte(src, charsNeeded - 1);
        return charsNeeded;
    }
}

struct QUtf8BaseTraits
{
    static const bool isTrusted = false;
    static const bool allowNonCharacters = true;
    static const bool skipAsciiHandling = false;
    static const int Error = -1;
    static const int EndOfString = -2;
    static bool isValidCharacter(uint u)
    {
        return int(u) >= 0;
    }
    static void appendByte(uchar *&ptr, uchar b)
    {
        *ptr++ = b;
    }
    static uchar peekByte(const uchar *ptr, int n = 0)
    {
        return ptr[n];
    }
    static qptrdiff availableBytes(const uchar *ptr, const uchar *end)
    {
        return end - ptr;
    }
    static void advanceByte(const uchar *&ptr, int n = 1)
    {
        ptr += n;
    }
    static void appendUtf16(ushort *&ptr, ushort uc)
    {
        *ptr++ = uc;
    }
    static void appendUcs4(ushort *&ptr, uint uc)
    {
        appendUtf16(ptr, QChar::highSurrogate(uc));
        appendUtf16(ptr, QChar::lowSurrogate(uc));
    }
    static ushort peekUtf16(const ushort *ptr, int n = 0)
    {
        return ptr[n];
    }
    static qptrdiff availableUtf16(const ushort *ptr, const ushort *end)
    {
        return end - ptr;
    }
    static void advanceUtf16(const ushort *&ptr, int n = 1)
    {
        ptr += n;
    }
    // it's possible to output to UCS-4 too
    static void appendUtf16(uint *&ptr, ushort uc)
    {
        *ptr++ = uc;
    }
    static void appendUcs4(uint *&ptr, uint uc)
    {
        *ptr++ = uc;
    }
};

inline uchar hexdig(uint u)
{
    return (u < 0xa ? '0' + u : 'a' + u - 0xa);
}

QByteArray escapedString(const QString &s)
{
    QByteArray ba(s.length(), Qt::Uninitialized);
    uchar *cursor = reinterpret_cast<uchar *>(const_cast<char *>(ba.constData()));
    const uchar *ba_end = cursor + ba.length();
    const ushort *src = reinterpret_cast<const ushort *>(s.constBegin());
    const ushort *const end = reinterpret_cast<const ushort *>(s.constEnd());
    while (src != end) {
        if (cursor >= ba_end - 6) {
            // ensure we have enough space
            int pos = cursor - reinterpret_cast<const uchar *>(ba.constData());
            ba.resize(ba.size() * 2);
            cursor = reinterpret_cast<uchar *>(ba.data()) + pos;
            ba_end = reinterpret_cast<const uchar *>(ba.constData()) + ba.length();
        }
        uint u = *src++;
        if (u < 0x80) {
            if (u < 0x20 || u == 0x22 || u == 0x5c) {
                *cursor++ = '\\';
                switch (u) {
                case 0x22:
                    *cursor++ = '"';
                    break;
                case 0x5c:
                    *cursor++ = '\\';
                    break;
                case 0x8:
                    *cursor++ = 'b';
                    break;
                case 0xc:
                    *cursor++ = 'f';
                    break;
                case 0xa:
                    *cursor++ = 'n';
                    break;
                case 0xd:
                    *cursor++ = 'r';
                    break;
                case 0x9:
                    *cursor++ = 't';
                    break;
                default:
                    *cursor++ = 'u';
                    *cursor++ = '0';
                    *cursor++ = '0';
                    *cursor++ = hexdig(u >> 4);
                    *cursor++ = hexdig(u & 0xf);
                }
            } else {
                *cursor++ = (uchar)u;
            }
        } else if (QUtf8Functions::toUtf8<QUtf8BaseTraits>(u, cursor, src, end) < 0) {
            // failed to get valid utf8 use JSON escape sequence
            *cursor++ = '\\';
            *cursor++ = 'u';
            *cursor++ = hexdig(u >> 12 & 0x0f);
            *cursor++ = hexdig(u >> 8 & 0x0f);
            *cursor++ = hexdig(u >> 4 & 0x0f);
            *cursor++ = hexdig(u & 0x0f);
        }
    }
    ba.resize(cursor - reinterpret_cast<const uchar *>(ba.constData()));
    return ba;
}

QJsonTreeItem* QJsonTreeItem::load(const QJsonValue& value, QJsonTreeItem* parent)
{
    QJsonTreeItem *rootItem = new QJsonTreeItem(parent);
    rootItem->setKey("root");

    if (value.isObject()) {
        //Get all QJsonValue childs
        const QStringList keys = value.toObject().keys(); // To prevent clazy-range warning
        for (const QString &key : keys) {
            QJsonValue v = value.toObject().value(key);
            QJsonTreeItem *child = load(v, rootItem);
            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);
        }
    } else if (value.isArray()) {
        //Get all QJsonValue childs
        int index = 0;
        const QJsonArray array = value.toArray(); // To prevent clazy-range warning
        for (const QJsonValue &v : array) {
            QJsonTreeItem *child = load(v, rootItem);
            child->setKey(QString::number(index));
            child->setType(v.type());
            rootItem->appendChild(child);
            ++index;
        }
    } else {
        rootItem->setValue(value.toVariant());
        rootItem->setType(value.type());
    }

    return rootItem;
}

//-----------------------------------------------------------------------------------------

QJsonTreeModel::QJsonTreeModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_rootItem{new QJsonTreeItem}
{
    m_headers.append("key");
    m_headers.append("type");
    m_headers.append("value");
}

QJsonTreeModel::QJsonTreeModel(const QString& fileName, QObject *parent) :
    QJsonTreeModel(parent)
{
    loadPath(fileName);
}

QJsonTreeModel::QJsonTreeModel(QIODevice *device, QObject *parent) :
    QJsonTreeModel(parent)
{
    loadFile(device);
}

QJsonTreeModel::QJsonTreeModel(const QByteArray& json, QObject *parent) :
    QJsonTreeModel(parent)
{
    loadJson(json);
}

QJsonTreeModel::~QJsonTreeModel()
{
    delete m_rootItem;
}

bool QJsonTreeModel::loadPath(const QString &fileName)
{
    QFile file(fileName);
    bool success = false;
    if (file.open(QIODevice::ReadOnly)) {
        success = loadFile(&file);
        file.close();
    } else {
        success = false;
    }

    return success;
}

bool QJsonTreeModel::loadFile(QIODevice *device)
{
    return loadJson(device->readAll());
}

bool QJsonTreeModel::loadJson(const QByteArray &json)
{
    QJsonParseError parseError;
    const QJsonDocument& jdoc = QJsonDocument::fromJson(json, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        beginResetModel();
        delete m_rootItem;
        m_rootItem = QJsonTreeItem::load(QJsonValue(QJsonDocument::fromJson((QString(
                                                                                 "{\"Error\":\"") + QString(parseError.errorString() +
                                                                               "\",\"offset\":")+ QString::number(parseError.offset) +
                                                                                "}").toUtf8()).object()));
        endResetModel();
        return true;
    }
    if (!jdoc.isNull())
    {
        beginResetModel();
        delete m_rootItem;
        if (jdoc.isArray())
        {
            m_rootItem = QJsonTreeItem::load(QJsonValue(jdoc.array()));
            m_rootItem->setType(QJsonValue::Array);

        }
        else
        {
            m_rootItem = QJsonTreeItem::load(QJsonValue(jdoc.object()));
            m_rootItem->setType(QJsonValue::Object);
        }
        endResetModel();
        return true;
    }

    qWarning()<<Q_FUNC_INFO<<"cannot load json";
    return false;
}


QVariant QJsonTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    QJsonTreeItem *item = static_cast<QJsonTreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return QString("%1").arg(item->key());

        if (index.column() == 1)
            return item->value();
    } else if (Qt::EditRole == role) {
        if (index.column() == 1)
            return item->value();
    }

    return {};
}

bool QJsonTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    if (Qt::EditRole == role) {
        if (col == 1) {
            QJsonTreeItem *item = static_cast<QJsonTreeItem*>(index.internalPointer());
            item->setValue(value);
            emit dataChanged(index, index, {Qt::EditRole});
            return true;
        }
    }

    return false;
}

QVariant QJsonTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        return m_headers.value(section);
    else
        return {};
}

QModelIndex QJsonTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    QJsonTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

    QJsonTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return {};
}

QModelIndex QJsonTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    QJsonTreeItem *childItem = static_cast<QJsonTreeItem*>(index.internalPointer());
    QJsonTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonTreeModel::rowCount(const QModelIndex &parent) const
{
    QJsonTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int QJsonTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

Qt::ItemFlags QJsonTreeModel::flags(const QModelIndex &index) const
{
    int col   = index.column();
    auto item = static_cast<QJsonTreeItem*>(index.internalPointer());

    auto isArray = QJsonValue::Array == item->type();
    auto isObject = QJsonValue::Object == item->type();

    if ((col == 1) && !(isArray || isObject))
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index);
}

QByteArray QJsonTreeModel::json()
{
    auto jsonValue = genJson(m_rootItem);
    QByteArray json;
    if (jsonValue.isNull())
        return json;

    if (jsonValue.isArray())
        arrayToJson(jsonValue.toArray(), json, 0, false);
    else
        objectToJson(jsonValue.toObject(), json, 0, false);

    return json;
}

void QJsonTreeModel::objectToJson(QJsonObject jsonObject, QByteArray &json, int indent, bool compact)
{
    json += compact ? "{" : "{\n";
    objectContentToJson(jsonObject, json, indent + (compact ? 0 : 1), compact);
    json += QByteArray(4 * indent, ' ');
    json += compact ? "}" : "}\n";
}
void QJsonTreeModel::arrayToJson(QJsonArray jsonArray, QByteArray &json, int indent, bool compact)
{
    json += compact ? "[" : "[\n";
    arrayContentToJson(jsonArray, json, indent + (compact ? 0 : 1), compact);
    json += QByteArray(4 * indent, ' ');
    json += compact ? "]" : "]\n";
}

void QJsonTreeModel::arrayContentToJson(QJsonArray jsonArray, QByteArray &json, int indent, bool compact)
{
    if (jsonArray.size() <= 0)
        return;

    QByteArray indentString(4 * indent, ' ');
    int i = 0;
    while (1) {
        json += indentString;
        valueToJson(jsonArray.at(i), json, indent, compact);
        if (++i == jsonArray.size()) {
            if (!compact)
                json += '\n';
            break;
        }
        json += compact ? "," : ",\n";
    }
}
void QJsonTreeModel::objectContentToJson(QJsonObject jsonObject, QByteArray &json, int indent, bool compact)
{
    if (jsonObject.size() <= 0)
        return;

    QByteArray indentString(4 * indent, ' ');
    int i = 0;
    while (1) {
        QString key = jsonObject.keys().at(i);
        json += indentString;
        json += '"';
        json += escapedString(key);
        json += compact ? "\":" : "\": ";
        valueToJson(jsonObject.value(key), json, indent, compact);
        if (++i == jsonObject.size()) {
            if (!compact)
                json += '\n';
            break;
        }
        json += compact ? "," : ",\n";
    }
}

void QJsonTreeModel::valueToJson(QJsonValue jsonValue, QByteArray &json, int indent, bool compact)
{
    QJsonValue::Type type = jsonValue.type();
    switch (type) {
    case QJsonValue::Bool:
        json += jsonValue.toBool() ? "true" : "false";
        break;
    case QJsonValue::Double: {
        const double d = jsonValue.toDouble();
        if (qIsFinite(d)) {
            json += QByteArray::number(d, 'f', QLocale::FloatingPointShortest);
        } else {
            json += "null"; // +INF || -INF || NaN (see RFC4627#section2.4)
        }
        break;
    }
    case QJsonValue::String:
        json += '"';
        json += escapedString(jsonValue.toString());
        json += '"';
        break;
    case QJsonValue::Array:
        json += compact ? "[" : "[\n";
        arrayContentToJson(jsonValue.toArray(), json, indent + (compact ? 0 : 1), compact);
        json += QByteArray(4 * indent, ' ');
        json += ']';
        break;
    case QJsonValue::Object:
        json += compact ? "{" : "{\n";
        objectContentToJson(jsonValue.toObject(), json, indent + (compact ? 0 : 1), compact);
        json += QByteArray(4 * indent, ' ');
        json += '}';
        break;
    case QJsonValue::Null:
    default:
        json += "null";
    }
}

QJsonValue  QJsonTreeModel::genJson(QJsonTreeItem * item) const
{
    auto type   = item->type();
    int  nchild = item->childCount();

    if (QJsonValue::Object == type) {
        QJsonObject jo;
        for (int i = 0; i < nchild; ++i) {
            auto ch = item->child(i);
            auto key = ch->key();
            jo.insert(key, genJson(ch));
        }
        return  jo;
    } else if (QJsonValue::Array == type) {
        QJsonArray arr;
        for (int i = 0; i < nchild; ++i) {
            auto ch = item->child(i);
            arr.append(genJson(ch));
        }
        return arr;
    } else {
        QJsonValue va;
        switch(item->value().type()) {
        case QVariant::Bool: {
            va = item->value().toBool();
            break;
        }
        default:
            va = item->value().toString();
            break;
        }
        (item->value());
        return va;
    }
}
