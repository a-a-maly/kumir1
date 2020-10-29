//
// C++ Interface: doubleoverflowchecker
//
// Description:
//
//
// Author: Виктор Яковлев <V.Yacovlev@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DOUBLEOVERFLOWCHECKER_H
#define DOUBLEOVERFLOWCHECKER_H

#include <QString>

/**
    @author Виктор Яковлев <V.Yacovlev@gmail.com>
 */
class DoubleOverflowChecker
{
public:
	/**
	 * Проверка операции сложения
	 * @param first первое слагаемое
	 * @param second второе слагаемое
	 * @return возможность выполнения операции
	 */
	static bool checkSumm(const double first, const double second);

	/**
	 * Проверка операции вычитания
	 * @param first уменьшаемое
	 * @param second вычитаемое
	 * @return возможность выполнения операции
	 */
	static bool checkDiff(const double first, const double second);

	/**
	 * Проверка операции умножения
	 * @param first первый множитель
	 * @param second второй множитель
	 * @return возможность выполнения операции
	 */
	static bool checkProd(const double first, const double second);

	/**
	 * Проверка операции возведения в неотрицательную степень
	 * @param first основание
	 * @param second показатель
	 * @return возможность выполнения операции
	 */
	static bool checkPower(const double first, const double second);

	/**
	 * Проверка операции преобразования из вещественного числа
	 * @param value строковое представление числа
	 * @return возможность выполнения операции
	 */
	static bool checkFromString(const QString &value);

	static double MAX_DOUBLE_VALUE;

private:
	static double log_2(const double x);
	static double power2in1023;
};

#endif
