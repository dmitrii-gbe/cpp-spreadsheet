#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <cmath>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;
const int PRE_A_POSITION = 64;
const int POST_Z_POSITION = 91;
const int PRE_0_POSITION = 48;
const int POST_9_POSITION = 57;

// Позиция ячейки. Индексация с нуля.
struct Position {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const {
        return row == rhs.row && col == rhs.col;
    }
    bool operator<(Position rhs) const {
        if (row < rhs.row){
            return true;
        }
        else {
            return col < rhs.col;
        }
    }

    bool IsValid() const {
        return this->col < MAX_COLS && this->row < MAX_ROWS && this->col >= 0 && this->row >= 0;
    }

    std::string ToString() const {
        if (!this->IsValid()){
            return std::string("");
        }
        const int letters_in_2nd = std::pow(LETTERS, 2);
        int a, b, c;
        a = col / letters_in_2nd;
        b = (col - letters_in_2nd * a) / LETTERS;
        c = (col - letters_in_2nd * a - LETTERS * b);
        if (a == 1 && b == 0){
            a = 0;
            b = LETTERS;
        }
        std::string result;
        if (a > 0){
            result += static_cast<char>(a + PRE_A_POSITION);
        }
        if (b > 0){
            result += static_cast<char>(b + PRE_A_POSITION);
        }
        result += static_cast<char>(c + PRE_A_POSITION + 1);
        result += std::to_string(row + 1);
        return result;
    }

    static Position FromString(std::string_view str) {
    std::string s;
        for (const auto& c : str){
            if (static_cast<int>(c) > PRE_A_POSITION && static_cast<int>(c) < POST_Z_POSITION){
                s += c;
                str.remove_prefix(1);
            }
        }
        int column = 0;
        int power = 0;
        for (int i = s.size() - 1; i >= 0; --i){
            column += (static_cast<int>(s[i]) - PRE_A_POSITION) * std::pow(LETTERS, power);
            ++power;
        }
        if (str.empty() || str.size() > 5){
            return Position::NONE;
        }
        for (const char c : str){
            if (static_cast<int>(c) > POST_9_POSITION || static_cast<int>(c) < PRE_0_POSITION){
                return Position::NONE;
            }
        }
        int row = std::stoi(std::string(str));
        Position result = {row - 1, column - 1};
        if (!result.IsValid()){
            return Position::NONE;
        }
        return result;
    }

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};


struct Size {
    int rows = 0;
    int cols = 0;

bool operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}
};


class FormulaError {
public:
    enum class Category {
        Ref,    // ссылка на ячейку с некорректной позицией
        Value,  // ячейка не может быть трактована как число
        Div0,  // в результате вычисления возникло деление на ноль
    };

    FormulaError(Category category) : category_(category)
    {
    }

    Category GetCategory() const {
        return category_;
    }

    bool operator==(FormulaError rhs) const {
        return category_ == rhs.category_;
    }

    std::string_view ToString() const {
        if (category_ == Category::Ref){
            return "#REF!";
        }
        if (category_ == Category::Value){
            return "#VALUE!";
        }
        if (category_ == Category::Div0){
            return "#DIV0!";
        }
    }

private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);

// Исключение, выбрасываемое при попытке задать синтаксически некорректную
// формулу
class FormulaException : public std::runtime_error {
public:
using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое при попытке передать в метод некорректную позицию
class InvalidPositionException : public std::out_of_range {
public:
using std::out_of_range::out_of_range;
};

// Исключение, выбрасываемое, если вставка строк/столбцов в таблицу приведёт к
// ячейке с позицией больше максимально допустимой
class TableTooBigException : public std::runtime_error {
public:
using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое при попытке задать формулу, которая приводит к
// циклической зависимости между ячейками
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline constexpr char FORMULA_SIGN = '=';
inline constexpr char ESCAPE_SIGN = '\'';

class CellInterface {
public:
// Либо текст ячейки, либо значение формулы, либо сообщение об ошибке из
// формулы
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() = default;

// Задаёт содержимое ячейки. Если текст начинается со знака "=", то он
// интерпретируется как формула. Уточнения по записи формулы:
// * Если текст содержит только символ "=" и больше ничего, то он не считается
// формулой
// * Если текст начинается с символа "'" (апостроф), то при выводе значения
// ячейки методом GetValue() он опускается. Можно использовать, если нужно
// начать текст со знака "=", но чтобы он не интерпретировался как формула.


//virtual void Set(std::string text) = 0;

// Возвращает видимое значение ячейки.
// В случае текстовой ячейки это её текст (без экранирующих символов). В
// случае формулы - числовое значение формулы или сообщение об ошибке.
    virtual Value GetValue() const = 0;
// Возвращает внутренний текст ячейки, как если бы мы начали её
// редактирование. В случае текстовой ячейки это её текст (возможно,
// содержащий экранирующие символы). В случае формулы - её выражение.
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

// Интерфейс таблицы
class SheetInterface {
public:
virtual ~SheetInterface() = default;

// Задаёт содержимое ячейки.
// * Если текст начинается с символа "'" (апостроф), то при выводе значения
// ячейки методом GetValue() он опускается. Можно использовать, если нужно
// начать текст со знака "=", но чтобы он не интерпретировался как формула.
virtual void SetCell(Position pos, std::string text) = 0;

// Возвращает значение ячейки.
// Если ячейка пуста, может вернуть nullptr.
virtual const CellInterface* GetCell(Position pos) const = 0;
virtual CellInterface* GetCell(Position pos) = 0;

// Очищает ячейку.
// Последующий вызов GetCell() для этой ячейки вернёт либо nullptr, либо
// объект с пустым текстом.
virtual void ClearCell(Position pos) = 0;

// Вычисляет размер области, которая участвует в печати.
// Определяется как ограничивающий прямоугольник всех ячеек с непустым
// текстом.
virtual Size GetPrintableSize() const = 0;

// Выводит всю таблицу в переданный поток. Столбцы разделяются знаком
// табуляции. После каждой строки выводится символ перевода строки. Для
// преобразования ячеек в строку используются методы GetValue() или GetText()
// соответственно. Пустая ячейка представляется пустой строкой в любом случае.
virtual void PrintValues(std::ostream& output) const = 0;
virtual void PrintTexts(std::ostream& output) const = 0;
};

// Создаёт готовую к работе пустую таблицу.
std::unique_ptr<SheetInterface> CreateSheet();