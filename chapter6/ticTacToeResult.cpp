#include <iostream>
#include <functional>
#include <numeric>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

using namespace std;
using namespace std::placeholders;

typedef vector<char> Line;
typedef vector<Line> Board;
typedef vector<Line> Lines;

template<typename DestinationType>
auto transformAll = [](const auto& source, auto lambda){
    DestinationType result;
    transform(source.begin(), source.end(), back_inserter(result), lambda);
    return result;
};

auto toRange = [](const auto& collection){
    vector<int> range(collection.size());
    iota(begin(range), end(range), 0);
    return range;
};

auto accumulateAll = [](const auto source, auto lambda){
    return accumulate(source.begin(), source.end(), typename decltype(source)::value_type(), lambda);
};

auto allOfCollection = [](const auto& collection, auto lambda){
    return all_of(collection.begin(), collection.end(), lambda);
};

auto any_of_collection = [](const auto& collection, auto lambda){
    return any_of(collection.begin(), collection.end(), lambda);
};

auto lineToString = [](const auto line){
    return transformAll<string>(line, [](auto const token) -> char { return token;});
};

auto boardToLinesString = [](const auto board){
    return transformAll<vector<string>>(board, lineToString);
};

auto boardToString = [](const auto board){
    auto linesAsString = boardToLinesString(board);
    return accumulateAll(linesAsString, 
            [](string current, string lineAsString) { return current + lineAsString + "\n"; }
            );
};

auto concatenate = [](auto first, const auto second){
    auto result(first);
    result.insert(result.end(), make_move_iterator(second.begin()), make_move_iterator(second.end()));
    return result;
};

auto concatenate3 = [](const auto first, const auto second, const auto third){
    return concatenate(concatenate(first, second), third);
};

typedef pair<int, int> Coordinate;

auto accessAtCoordinates = [](const auto board, const Coordinate coordinate){
    return board[coordinate.first][coordinate.second];
};

auto projectCoordinates = [](const auto board, const auto coordinates){
    auto boardElementFromCoordinates = bind(accessAtCoordinates, board, _1);
    return transformAll<Line>(coordinates, boardElementFromCoordinates);
};

auto mainDiagonalCoordinates = [](const auto board){
    auto range = toRange(board);
    return transformAll<vector<Coordinate>>(range, [](auto index){return make_pair(index, index);});
};

auto secondaryDiagonalCoordinates = [](const auto board){
    auto range = toRange(board);
    return transformAll<vector<Coordinate>>(range, [board](auto index){return make_pair(index, board.size() - index - 1);});
};

auto columnCoordinates = [](const auto board, const auto columnIndex){
    auto range = toRange(board);
    return transformAll<vector<Coordinate>>(range, [columnIndex](auto index){return make_pair(index, columnIndex);});
};

auto lineCoordinates = [](const auto board, const auto lineIndex){
    auto range = toRange(board);
    return transformAll<vector<Coordinate>>(range, [lineIndex](auto index){return make_pair(lineIndex, index);});
};

auto mainDiagonal = [](const auto board){
    return projectCoordinates(board, mainDiagonalCoordinates(board));
};

auto secondaryDiagonal = [](const auto board){
    return projectCoordinates(board, secondaryDiagonalCoordinates(board));
};

auto column = [](const auto board, const auto columnIndex){
    return projectCoordinates(board, columnCoordinates(board, columnIndex));
};

auto line = [](const auto board, const int lineIndex){
    return projectCoordinates(board, lineCoordinates(board, lineIndex));
};

auto allLines = [](const auto board) {
    auto range = toRange(board);
    return transformAll<Lines>(range, [board](const auto index) { return line(board, index);});
};

auto allColumns = [](const auto board) {
    auto range = toRange(board);
    return transformAll<Lines>(range, [board](const auto index) { return column(board, index);});
};

auto allDiagonals = [](const auto board) -> Lines {
    return {mainDiagonal(board), secondaryDiagonal(board)};
};

template<typename SourceType, typename DestinationType>
auto applyAllLambdasToValue = [](const auto lambdas, const auto value){
    return transformAll<DestinationType>(lambdas, [value](const auto lambda){ return lambda(value); } );
};

auto allLinesColumnsAndDiagonals = [](const auto board) {
    return concatenate3(allLines(board), allColumns(board), allDiagonals(board));
};

auto lineFilledWith = [](const auto line, const auto tokenToCheck){
    return allOfCollection(line, [&tokenToCheck](const auto token){ return token == tokenToCheck;});
};

auto lineFilledWithX = bind(lineFilledWith, _1, 'X'); 
auto lineFilledWithO = bind(lineFilledWith, _1, 'O');

auto xWins = [](const auto& board){
    return any_of_collection(allLinesColumnsAndDiagonals(board), lineFilledWithX);
};

auto oWins = [](const auto& board){
    return any_of_collection(allLinesColumnsAndDiagonals(board), lineFilledWithO);
};

auto noneOf = [](const auto& collection, auto lambda){
    return none_of(collection.begin(), collection.end(), lambda);
};

auto isEmpty = [](const auto& token){return token == ' ';};
auto fullLine = [](const auto& line){
    return noneOf(line, isEmpty);
};

auto full = [](const auto& board){
    return allOfCollection(board, fullLine);
};

auto draw = [](const auto& board){
    return full(board) && !xWins(board) && !oWins(board); 
};

auto inProgress = [](const auto& board){
    return !full(board) && !xWins(board) && !oWins(board); 
};

auto findInCollection = [](const auto& collection, auto lambda){
    auto result = find_if(collection.begin(), collection.end(), lambda);
    return (result == collection.end()) ? nullopt : optional(*result);
};

auto findInCollectionWithDefault = [](const auto& collection, const auto& defaultResult, const auto& lambda){
    const auto result = findInCollection(collection, lambda);
    return result.has_value() ? (*result) : defaultResult;
}; 

auto howDidXWin = [](const auto& board){
    map<string, Line> linesWithDescription = {
        {"first line", line(board, 0)},
        {"second line", line(board, 1)},
        {"last line", line(board, 2)},
        {"first column", column(board, 0)},
        {"second column", column(board, 1)},
        {"last column", column(board, 2)},
        {"main diagonal", mainDiagonal(board)},
        {"secondary diagonal", secondaryDiagonal(board)},
        {"diagonal", secondaryDiagonal(board)},
    };
    const auto xDidNotWin = make_pair("X did not win", Line());
    const auto xWon = [](const auto value){
        return lineFilledWithX(value.second);
    };

    return findInCollectionWithDefault(linesWithDescription, xDidNotWin, xWon).first; 
};

TEST_CASE("lines"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Line expectedLine0 = {'X', 'X', 'X'};
    CHECK_EQ(expectedLine0, line(board, 0));
    Line expectedLine1 = {' ', 'O', ' '};
    CHECK_EQ(expectedLine1, line(board, 1));
    Line expectedLine2 = {' ', ' ', 'O'};
    CHECK_EQ(expectedLine2, line(board, 2));
}

TEST_CASE("all columns"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Line expectedColumn0 = {'X', ' ', ' '};
    CHECK_EQ(expectedColumn0, column(board, 0));
    Line expectedColumn1 = {'X', 'O', ' '};
    CHECK_EQ(expectedColumn1, column(board, 1));
    Line expectedColumn2 = {'X', ' ', 'O'};
    CHECK_EQ(expectedColumn2, column(board, 2));
}

TEST_CASE("main diagonal"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Line expectedDiagonal = {'X', 'O', 'O'};

    CHECK_EQ(expectedDiagonal, mainDiagonal(board));
}

TEST_CASE("secondary diagonal"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Line expectedDiagonal = {'X', 'O', ' '};

    CHECK_EQ(expectedDiagonal, secondaryDiagonal(board));
}


TEST_CASE("all lines, columns and diagonals"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Lines expected = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'},
        {'X', ' ', ' '},
        {'X', 'O', ' '},
        {'X', ' ', 'O'},
        {'X', 'O', 'O'},
        {'X', 'O', ' '}
    };

    auto all = allLinesColumnsAndDiagonals(board);
    CHECK_EQ(expected, all);
}

TEST_CASE("line to string"){
    Line line = {
        ' ', 'X', 'O'
    };

    CHECK_EQ(" XO", lineToString(line));
}

TEST_CASE("board to lines string"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };
    vector<string> expected = {
        "XXX",
        " O ",
        "  O"
    };

    CHECK_EQ(expected, boardToLinesString(board));
}

TEST_CASE("board to string"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };
    string expected = "XXX\n O \n  O\n";

    CHECK_EQ(expected, boardToString(board));
}

TEST_CASE("Line filled with X"){
    Line line = {'X', 'X', 'X'};

    CHECK(lineFilledWithX(line));
}

TEST_CASE("Line not filled with X"){
    CHECK(!lineFilledWithX(Line({'X', 'O', 'X'})));
    CHECK(!lineFilledWithX(Line({'X', ' ', 'X'})));
}

TEST_CASE("X wins"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    CHECK(xWins(board));
}

TEST_CASE("O wins"){
    Board board = {
        {'X', 'O', 'X'},
        {' ', 'O', ' '},
        {' ', 'O', 'X'}
    };

    CHECK(oWins(board));
}

TEST_CASE("draw"){
    Board board = {
        {'X', 'O', 'X'},
        {'O', 'O', 'X'},
        {'X', 'X', 'O'}
    };

    CHECK(draw(board));
}

TEST_CASE("in progress"){
    Board board = {
        {'X', 'O', 'X'},
        {'O', ' ', 'X'},
        {'X', 'X', 'O'}
    };

    CHECK(inProgress(board));
}


TEST_CASE("how did X win"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    CHECK_EQ("first line", howDidXWin(board));
}

TEST_CASE("X did not win"){
    Board board = {
        {'X', 'X', ' '},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    CHECK_EQ("X did not win", howDidXWin(board));
}

TEST_CASE("Project column"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    Line expected0 = {'X', ' ', ' '};
    CHECK_EQ(expected0, column(board, 0));
    Line expected1 = {'X', 'O', ' '};
    CHECK_EQ(expected1, column(board, 1));
    Line expected2 = {'X', ' ', 'O'};
    CHECK_EQ(expected2, column(board, 2));
}

TEST_CASE("Range"){
    Board board = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };

    vector<int> expected = {0, 1, 2};
    CHECK_EQ(expected, toRange(board));
    CHECK_EQ(expected, toRange(board[0]));
}
