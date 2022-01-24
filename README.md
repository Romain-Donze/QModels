QModels
==============

A set of models extending the Qt model-view framework

Installation
------------
1. clone or download this repository
2. add `include  (<path/to/QModels>/QModels.pri)` in your `.pro`
3. `import Models 1.0` to use this library in your QML files


# QObjectListModel

`QObjectListModel` is a hard-fork of QOlm * [Olivier LDff](https://github.com/OlivierLDff)

`QObjectListModel` object based on `QAbstractListModel` that provide a list of `QObject` based class to **qml** and **c++**. The model dynamically update views by reacting to **insert**, **remove**, **move** operations.


`QObjectListModel` is based on `QAbstractListModel`, and behave as a list of custom `QObject`. `QObjectModelBase` is detail implementation to provide `signals` and `slots` that `moc` can handle. Since `moc` isn't working with template class.

### C++ Getting started

Most of the time you want to store more than just `QObject` type, so create your custom type.

```cpp
#include <QtCore/QObject>

class Foo : public QObject
{
    Q_OBJECT
public:
    Foo(QObject* parent = nullptr) : QObject(parent) {}

    int foo = 0;
};
```

Then use a `QObjectListModel` based object with class inheritance or as a typedef.

```cpp
#include <Foo.h>
#include <QObjectListModel/QObjectListModel.h>

// Declare type
using FooList = QObjectListModel<Foo>;

// Or create custom class
class FooList : public QObjectListModel<Foo>
{
    Q_OBJECT
public:
    Foo(QObject* parent = nullptr,
        const QList<QByteArray> & exposedRoles = {},
        const QByteArray & displayRole = {}):
    QObjectListModel<Foo>::(parent, exposedRoles, displayRole)
    {
    }
};
```

Then simply use it as a regular list.

#### Insert elements

The object provide multiple way to insert an object in the list:

* `append` : Add an object at the end of the list.
* `prepend` : Add an object at the beginning of the list.
* `insert` : Insert object at requested offset.

Those three functions can also take a `QList<_Object*>` as a parameter entry to insert multiple object at once.

```cpp
FooList list;
Foo foo1;
Foo foo2;
Foo foo3;
Foo foo4;

// {foo1}
list.append(&foo1);

// {foo2, foo1}
list.prepend(&foo2);

// {foo2, foo1, foo3}
list.append(&foo3);

// {foo2, foo4, foo1, foo3}
list.insert(1, &foo4);
```

#### Remove elements

To remove an item, simply call the `remove` function, either with a pointer to the `_Object*` , or with the index of the object at which you want to remove.

All elements can also be removed using `clear` function.

```cpp
FooList list;
Foo foo1;
Foo foo2;
Foo foo3;
Foo foo4;
list.append({&foo1, &foo2, &foo3, &foo4});

// { &foo1, &foo2, &foo4 }
list.remove(&foo3);
// { &foo1, &foo2 }
list.remove(2);
// Remove all elements.
list.clear();
```

#### Move elements

Elements can be moved within the list, without changing the list size.

* `move` from object at index `from` to index `to`.
* `moveUp`: Move object at index `index` to `index-1`. This function make sense when seeing the list in a `ListView` for example. It move the item to previous index.
* `moveDown`: Move object from `index` to `index+1`. This function make sense in a `ListView` in a column. It move the item to next index.
* `moveNext`: alias of `moveDown`.
* `movePrevious`: alias of `moveUp`.

```cpp
FooList list;
Foo foo1;
Foo foo2;
Foo foo3;
Foo foo4;
list.append({&foo1, &foo2, &foo3, &foo4});

// { &foo1, &foo3, &foo4, &foo2 }
list.move(1, 3);

// { &foo2, &foo1, &foo3, &foo4 }
list.move(3, 0);

// { &foo2, &foo3, &foo1, &foo4 }
list.movePrevious(2);

// { &foo2, &foo3, &foo4, &foo1 }
list.moveNext(2);
```

#### Access element and get index

Multiple accessors can be used to get data.

* `get` : Get pointer to the `_Object*` at `index`.
* `indexOf` : Get the `index` from a `_Object*`
* `contains` : Get if a `_Object*` is present.
* `size`: Give the number of objects in the model
* `empty` : True if model is empty.

#### Object ownership

The library follow qt ownership rules. So when inserting an object without parent, the list take ownership of that object. When the same object is removed it will be `deleteLater`.

```c++
FooList list;
// list take ownership on new Foo
list.append(new Foo());
// Since FooList have ownership on the foo at index 0, it call deleteLater on it. No need to worry about memory management.
list.remove(0);
```

#### Observe list

##### Observe as QAbstractItemModel

The `QObjectListModel` derived object can be observe for insertion and deletion like any qt model.

* **[rowsAboutToBeInserted](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsAboutToBeInserted)**
* **[rowsAboutToBeMoved](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsAboutToBeMoved)**
* **[rowsAboutToBeRemoved](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsAboutToBeRemoved)**
* **[rowsInserted](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsInserted)**
* **[rowsMoved](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsMoved)**
* **[rowsRemoved](https://doc.qt.io/qt-5/qabstractitemmodel.html#rowsRemoved)**

But those signals are not very convenient to use as a end user. That's why `QObjectListModel` provide other way to observe the list.

##### Observe thru signals

`QObjectListModel::QObjectModelBase` provide basic signal to react to `QObject` insert/remove/move operation.

* `objectInserted(QObject* object, int index)`
* `objectRemoved(QObject* object, int index)`
* `objectMoved(QObject* object, int from, int to)`

They are call when the model can safely be iterated. You can simply retrieve a correct pointer by using `qobject_cast<_Object*>(object)`.

##### Function override observe.

Sometime it can be useful to do some processing before the whole world gets notify about our object operation. This method is only available if you define a custom list type.

```cpp
#include <Foo.h>
#include <QObjectListModel/QObjectListModel.h>

class FooList : public QObjectListModel<Foo>
{
    Q_OBJECT
public:
    FooList(QObject* parent = nullptr,
        const QList<QByteArray> & exposedRoles = {},
        const QByteArray & displayRole = {}):
    QObjectListModel<Foo>(parent, exposedRoles, displayRole)
    {
    }

protected:
    bool onObjectAboutToBeInserted(Foo* item, int row) override
    {
        // Item is not yet inserted, do some preinsert operation on it.
        // if return false, item won't be inserted
    }
    void onObjectInserted(Foo* item, int row) override
    {
        // Item just got inserted, but no callback/signal have been called yet.
    }
    bool onObjectAboutToBeMoved(Foo* item, int src, int dest) override
    {
        // Item haven't move yet, and no callback/signal have been called yet
        // if return false, item won't be moved
    }
    void onObjectMoved(Foo* item, int src, int dest) override
    {
        // Item have been moved. No Callback/Signal have been called yet.
    }
    bool onObjectAboutToBeRemoved(Foo* item, int row) override
    {
        // Item isn't removed yet, and no callback/signal have been called yet
        // if return false, item won't be removed
    }
    void onObjectRemoved(Foo* item, int row) override
    {
        // Item have been removed. Callback/Signal have been called
    }
};
```

##### Observe with callback

QObjectListModel provide lambda connection with already `qobject_cast` objects. This is the preferred and easier way to observe the list

```cpp
FooList list;

// Preferred API, safer to use when giving a context
list.onInserted(&list, [](Foo* foo, int index){});
list.onInserted(&list, [](Foo* foo){});
list.onRemoved(&list, [](Foo* foo, int index){});
list.onRemoved(&list, [](Foo* foo){});
list.onMoved(&list, [](Foo* foo, int from, int to){});

// Should only be used when your callback doesn't require any context
list.onInserted([](Foo* foo, int index){});
list.onInserted([](Foo* foo){});
list.onRemoved([](Foo* foo, int index){});
list.onRemoved([](Foo* foo){});
list.onMoved([](Foo* foo, int from, int to){});
```

> When connecting without any `receiver`, this list is used as the context.

#### Iterator

`QObjectListModel` is compatible with modern iterator, you can simply do:

```cpp
FooList list;
for(const auto* foo : list)
{
    //foo->getFoo()
}
for(auto* foo : list)
{
    //foo->setFoo(12)
}
```

### Getting Started Qml

The same api as the c++ work in qml. Every `Q_PROPERTY` are exposed as role, and another role `qtObject` allow to access the `QObject*`.

For the following example to work `Foo` and `FooList` need to be registered to the qml system.

```js
import QtQuick 2.0
import MyFoo 1.0

ListView {
    width: 180; height: 200
    FooList { id: _fooList }

    model: _fooList
    delegate: Text
    {
        // Access role qtObject and cast it to our type
        property Foo fooObj : model.qtObject
        text: index + ": " +
            fooObj.foo + // Access via casted object
            + ", " +
            foo // Access via role
    }

    Component.onCompleted:
    {
        _fooList.append(new Foo())
        _fooList.insert(1, new Foo())
        _fooList.prepend(new Foo())
    }
}
```

If you need to filter exposed roles, then use the constructor arguments. Same to set a display role.

```cpp
//QObjectListModel(QObject* parent = nullptr,
//   const QList<QByteArray> & exposedRoles = {},
//   const QByteArray & displayRole = {})

// The following code expose foo as exposedRoles, and foo as Qt::DisplayRole
FooList list(nullptr, {"foo"}, "foo");
```

It is recommended to only expose role that are required for `QSortFilterProxyModel ` subclass. And use native signal to property for property that often change.

# QObjectListProperty

```cpp
#include <QtCore/QObject>

class FooChild : public QObject
{
    Q_OBJECT
public:
    FooChild(QObject* parent = nullptr) : QObject(parent) {}

    int fooChild = 0;
};


class Foo : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QQmlListProperty<FooChild> childs READ getChilds CONSTANT)
public:
    Foo(QObject* parent = nullptr) : QObject(parent) {}
    
    const QObjectListProperty<FooChild> &getChilds() const 
    { 
        return m_childs; 
    }

    QObjectListProperty<FooChild> m_childs;
};
```

# QModelHelper
`QModelHelper` is a hard-fork of QmlModelHelper * [oKcerG](https://github.com/oKcerG/QmlModelHelper)

Access your models from QML or C++ without views or delegates

To use the QModelHelper, use it as an attached object of a model (it must be a subclass of QAbstractItemModel, meaning it can originate from c++ or be a ListModel from QML). Or as a wrapper in C++.

### Attached properties:
##### count, length, size : int
These properties holds the number of rows in the model.

##### isEmpty : int
This property holds a boolean refering if the model is empty or not (emptyChanged is only emitted when count goes from 0 to x or x to 0.

### Attached methods
##### object map(int row, int column = 0, QModelIndex parent = {})
Returns a live object mapping an index's data of the model. This is an object with read-write properties named as the model's roles.
If the index doesn't exist in the model, the properties will be `undefined`.
```
ComboBox {
    id: control
    model: vehiclesModel
    readonly property QtObject currentData: vehiclesModel.ModelHelper.map(control.currentIndex)
    contentItem: RowLayout {
        id: contentItem
        Image { source: control.currentData.imagePath }
        Label { text: control.currentData.name }
    }
    delegate: ItemDelegate {
        contentItem: RowLayout {
            Image { source: model.imagePath }
            Label { text: model.name }
        }
        highlighted: control.highlightedIndex == index
    }
}
```

##### int roleForName(string roleName)
Returns the role number for the given `roleName`. If no role is found for this name, `-1` is returned.

##### string roleName(int role)
Returns the role name for the given `role`. If no role name is found for this role, an empty string is returned.

##### var get(int row)
Return the item at `row` in the model as a map of all its roles.

This can be used in imperative code when you don't need the live updating provided by the `map` function. Only reading is supported.

##### var getProperty(int row, string roleName)
Return the data for the given `roleName` of the item at `row` in the model.

Similar to `get(int row)` except that it queries only one role. Prefer this one if you don't need multiple roles for a row.

##### var set(int row, var values)
Changes the values at `row` in the list model with the values in `values`. Properties not appearing in newValues are left unchanged.

##### bool setProperty(int row, string property, var value)
Changes the `property` at row in the list model to `value`.

##### int indexOf(string columnName, var value)
Returns the first row index where value at `columnName` equals `value`.

##### bool contains(string columnName, var value)
Returns true if a row with the value at `columnName` equals `value`. Similar to `int indexOf()`

# QVariantListModel

A dead-simple way to create a dynamic C++ list of any type and expose it to QML with the strong API of QAbstractListModel, way better than using a QVariantList property.
The type object is accessed with the `modelData` roleName.

# QConcatenateProxyModel

Work in progress

Concatenates rows from multiple source models

# QExtraColumnsProxyModel

TODO
Adds columns after existing columns

# QSelectableProxyModel 

TODO
A simplified `QExtraColumnsProxyModel` to add just a `selected` role column to a source model
