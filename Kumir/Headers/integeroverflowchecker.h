//
// C++ Interface: integeroverflowchecker
//
// Description:
//
//
// Author: Виктор Яковлев <V.Yacovlev@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef INTEGEROVERFLOWCHECKER_H
#define INTEGEROVERFLOWCHECKER_H

#include <QString>

/**
    @author Виктор Яковлев <V.Yacovlev@gmail.com>
*/
class IntegerOverflowChecker
{
public:
	/**
	 * 32 или 32/64-битная архитектура
	 */
	enum Architecture { Always32Bit, SystemDependent };

	/**
	 * Установка типа архитектуры
	 * @param arch тип архитектуры
	 */
	static void setArchitecture(Architecture arch)
	{
		m_architecture = arch;
		updateN();
	}

	/**
	 * Возвращает тип архитектуры
	 * @return тип архитектуры
	 */
	static Architecture architecture()
	{
		return m_architecture;
	}

	/**
	 * Проверка операции сложения
	 * @param first первое слагаемое
	 * @param second второе слагаемое
	 * @return возможность выполнения операции
	 */
	static bool checkSumm(const int first, const int second);

	/**
	 * Проверка операции вычитания
	 * @param first уменьшаемое
	 * @param second вычитаемое
	 * @return возможность выполнения операции
	 */
	static bool checkDiff(const int first, const int second);

	/**
	 * Проверка операции умножения
	 * @param first первый множитель
	 * @param second второй множитель
	 * @return возможность выполнения операции
	 */
	static bool checkProd(const int first, const int second);

	/**
	 * Проверка операции возведения в неотрицательную степень
	 * @param first основание
	 * @param second показатель
	 * @return возможность выполнения операции
	 */
	static bool checkPower(const int first, const uint second);

	/**
	 * Проверка операции преобразования из вещественного числа
	 * @param value вещественное значение
	 * @return возможность выполнения операции
	 */
	static bool checkFromReal(const qreal value);

	/**
	 * Проверка операции преобразования из вещественного числа
	 * @param value строковое представление числа
	 * @return возможность выполнения операции
	 */
	static bool checkFromString(const QString &value);

private:
	static void updateN();
	static Architecture m_architecture;
	static uint n;
	static qreal log_2(const qreal x);
	static int ipow(int base, uint pwr);
	static int power2inNminus2;
};

#endif
