/*==================[inclusions]=============================================*/
#define MAX_DIGITOS_EN_VALORES 7
#define MAX_PULSOS_AVANCE 46080                  //maxima cantidad de pylsos

#define DERECHA 1
#define IZQUIERDA 0
#define HOME_IZQ GPIO6
#define HOME_DER GPIO5
#define START GPIO7
#define STOP GPIO8
#include "sapi.h"       // <= sAPI header
#include "LCD_I2C_PCF8574.h"
//#include "LCD_i2c.h"

/*==================[macros and definitions]=================================*/
CONSOLE_PRINT_ENABLE
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/**
 * Estados de la maquina Bobinadora
 */
typedef enum {
	MENSAJE_BIENVENIDA,     //!< Mensaje inicial de Bienvenida
	MENU_CONFIG_GRAL,       //!< Estado de presentacion de menus
	CONTROL_VUELTAS,        //!< Estado de control de motores y sensor
//	CONTROL_MOTOR_BOBINADOR,        //!< CONTROL_MOTOR_BOBINADOR
//	CONTROL_AVANCE_ALAMBRE, //!< CONTROL_AVANCE_ALAMBRE
	REPETIR_BOBINADO,       //!< Estado de repeticion de bobinado previo
	SUB_MENU_CONFIG         //!< Estado de sub menu -no implementado-
} bob_state_t;
/**
 * Estados del menu de configuracion
 */

typedef enum {
	MENU_DIAMETRO_ALAMBRE,      //!< Estado de Menu de ingreso diametro alambre
	MENU_LONGITUD_CARRETE,   //!< Estado de Menu de ingreso longitud del carrete
	MENU_CANTIDAD_ESPIRAS,    //!< Estado de Menu de ingreso cantidad de espiras
//	MENU_CANTIDAD_CAPAS,        //!< Estado de Menu de ingresoMENU_CANTIDAD_CAPAS
	MENU_VELOCIDAD, //!< Estado de Menu de ingreso velocidad PWM inicial bobinado
	MENU_HOME_SET_PRIMERA_ESPIRA             //!< MENU_HOME_SET_PRIMERA_ESPIRA
} menu_config_t;

delay_t Delay100ms;
delay_t Delay1, Delay2, Delay3;

bob_state_t BobinadorState;
menu_config_t MenuConfigState; //MENU_DIAMETRO_ALAMBRE;
uint16_t diametro = 0;
uint16_t longitud = 0;
uint16_t cantidad_espiras = 0;
uint16_t cantidad_capas = 0;
uint16_t velocidad = 0;
uint8_t valorPWM = 50;
char conv_table[] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9',
		'C', '*', '0', '#', ',' };
// Teclado
keypad_t keypad;

// Filas --> Salidas
uint8_t keypadRowPins1[4] = { RS232_TXD, // Row 0
		CAN_RD,    // Row 1
		CAN_TD,    // Row 2
		T_COL1     // Row 3
		};

// Columnas --> Entradas con pull-up (MODO = GPIO_INPUT_PULLUP)
uint8_t keypadColPins1[4] = { T_FIL0,    // Column 0
		T_FIL3,    // Column 1
		T_FIL2,    // Column 2
		T_COL0     // Column 3
		};

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
void BobinadoraMEFInit(void);
void BobinadoraMEFUpdate(void);
void MensajeBienvenida(void);
void MenuConfigGeneral(void);
void IniciaBobinado(void);
void RepetirBobinado(void);
void ControlMotorPAP(uint32_t vel_paso, bool_t dir_giro);
void ControlMotorBobinador(uint8_t Duty);
bool_t LecturaSensor(void);
void pwmInit(void);
void SoftStartMotorBobinador(void);
uint16_t char2int(char *array, uint8_t n);
uint16_t KeypadToInt(uint16_t teclaPresionada);
/*==================[external functions definition]==========================*/

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */

int main(void) {

	/* ------------- INICIALIZACIONES ------------- */

	/* Inicializar la placa */

	boardConfig();
	i2cConfig(I2C0, 100000);
	tickConfig(1, 0);
	delayConfig(&Delay100ms, 100);
	delayConfig(&Delay1, 40);
	delayConfig(&Delay2, 40);
	delayConfig(&Delay3, 250);
	keypadConfig(&keypad, keypadRowPins1, 4, keypadColPins1, 4);
	gpioConfig(GPIO1, GPIO_OUTPUT);
	gpioConfig(GPIO0, GPIO_OUTPUT);
	gpioConfig(GPIO5, GPIO_INPUT_PULLUP);
	gpioConfig(GPIO6, GPIO_INPUT_PULLUP);
	gpioConfig(GPIO7, GPIO_INPUT_PULLUP);
	gpioConfig(GPIO8, GPIO_INPUT_PULLUP);
	adcConfig(ADC_ENABLE);
	consolePrintSetUart(UART_USB);consolePrintConfigUart(UART_USB, 115200);
	LCD_Init();
	pwmInit();
	BobinadoraMEFInit();

	/* ------------- REPETIR POR SIEMPRE ------------- */

	while (1) {
		BobinadoraMEFUpdate();
	}

	/* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado
	 *        por ningun S.O. */
	return 0;
}
/**
 * Inicializa la MEF de la bobinadora
 */
void BobinadoraMEFInit(void) {
	BobinadorState = MENSAJE_BIENVENIDA;
	//BobinadorState = MENU_CONFIG_GRAL;
	//BobinadorState = CONTROL_VUELTAS;
	MenuConfigState = MENU_DIAMETRO_ALAMBRE;
}
/**
 * Actualiza el estado de la MEF de la bobinadora
 */

void BobinadoraMEFUpdate(void) {

	switch (BobinadorState) {
	case MENSAJE_BIENVENIDA:
		MensajeBienvenida();
		delayInaccurate(3000);
		BobinadorState = MENU_CONFIG_GRAL;
		break;
	case MENU_CONFIG_GRAL:
		MenuConfigGeneral();
		break;
	case CONTROL_VUELTAS:
		IniciaBobinado();
		break;
	case REPETIR_BOBINADO:
		RepetirBobinado();
		break;

	}

}
/**
 * Mensaje inicial de bienvenida al sistema
 */
void MensajeBienvenida(void) {
	unsigned char Autoscroll[] = "Freigeiro - Lopez";
	unsigned char * p;
	p = Autoscroll;
	LCDclear();
	LCDbacklight();
	LCDcursorOff();
	LCDblinkOff();
	LCDsetCursor(1, 0);
	LCD_Write_Str("BOBINADORA LATERAL");
	LCDsetCursor(4, 1);
	LCD_Write_Str("Version 0.1");
	LCDsetCursor(6, 2);
	LCD_Write_Str("Autores:");
	LCDsetCursor(1, 3);
	while (*p) {
		LCD_Write_Char((char) *p++);
		LCDnoBacklight();
		delayInaccurate(100);
		LCDbacklight();
		delayInaccurate(100);
	}
}

void MenuConfigGeneral(void) {
	static uint16_t tecla = 0;
	static uint8_t digito = 0;
	static bool_t FlagUnaVez = FALSE;
	static bool_t FlancoDebounce = FALSE;
	static bool_t Flanco1 = FALSE, Flanco2 = FALSE;
	switch (MenuConfigState) {
	case MENU_DIAMETRO_ALAMBRE: {
		if (FlagUnaVez == FALSE) {
			LCDclear();
			LCDsetCursor(0, 0);
			LCD_Write_Str("Ingrese diametro del");
			LCDsetCursor(0, 1);
			LCD_Write_Str("alambre en mm:");
			LCDsetCursor(0, 2);
			LCD_Write_Str("Diametro:?");
			LCDsetCursor(1, 3);
			LCD_Write_Str("(# para continuar)");
			LCDsetCursor(9, 2);
			LCDblinkOn();
			FlagUnaVez = TRUE;
		}

		if (keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == FALSE) {
			FlancoDebounce = TRUE;
			diametro = KeypadToInt(tecla);
			if (conv_table[tecla] == '#') {
				FlagUnaVez = FALSE;
				digito = 0;
				MenuConfigState = MENU_LONGITUD_CARRETE;
			}
			if (digito < MAX_DIGITOS_EN_VALORES && conv_table[tecla] != '#')
				LCD_Write_Char(conv_table[tecla]);
		}

		if (!keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == TRUE) {
			FlancoDebounce = FALSE;
		}
	}
		break;
	case MENU_LONGITUD_CARRETE: {
		if (FlagUnaVez == FALSE) {
			LCDclear();
			LCDsetCursor(0, 0);
			LCD_Write_Str("Ingrese largo del");
			LCDsetCursor(0, 1);
			LCD_Write_Str("carrete en mm:");
			LCDsetCursor(0, 2);
			LCD_Write_Str("Largo:?");
			LCDsetCursor(1, 3);
			LCD_Write_Str("(# para continuar)");
			LCDsetCursor(6, 2);
			LCDblinkOn();
			FlagUnaVez = TRUE;
		}

		if (keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == FALSE) {
			FlancoDebounce = TRUE;
			longitud = KeypadToInt(tecla);
			if (conv_table[tecla] == '#') {
				FlagUnaVez = FALSE;
				digito = 0;
				MenuConfigState = MENU_CANTIDAD_ESPIRAS;
			}
			if (digito < MAX_DIGITOS_EN_VALORES && conv_table[tecla] != '#')
				LCD_Write_Char(conv_table[tecla]);
		}
		if (!keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == TRUE) {
			FlancoDebounce = FALSE;
		}
	}
		break;
	case MENU_CANTIDAD_ESPIRAS: {
		if (FlagUnaVez == FALSE) {
			LCDclear();
			LCDsetCursor(0, 0);
			LCD_Write_Str("Ingrese cantidad de");
			LCDsetCursor(0, 1);
			LCD_Write_Str("Espiras:");
			LCDsetCursor(0, 2);
			LCD_Write_Str("Cantidad:?");
			LCDsetCursor(1, 3);
			LCD_Write_Str("(# para continuar)");
			LCDsetCursor(9, 2);
			LCDblinkOn();
			FlagUnaVez = TRUE;
		}

		if (keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == FALSE) {
			FlancoDebounce = TRUE;
			cantidad_espiras = KeypadToInt(tecla);
			if (conv_table[tecla] == '#') {
				FlagUnaVez = FALSE;
				digito = 0;
				//MenuConfigState = MENU_CANTIDAD_CAPAS;
				MenuConfigState = MENU_VELOCIDAD;
			}
			if (digito < MAX_DIGITOS_EN_VALORES && conv_table[tecla] != '#')
				LCD_Write_Char(conv_table[tecla]);
		}
		if (!keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == TRUE) {
			FlancoDebounce = FALSE;
		}
	}
		break;
//	case MENU_CANTIDAD_CAPAS: {
//		if (FlagUnaVez == FALSE) {
//			LCDclear();
//			LCDsetCursor(0, 0);
//			LCD_Write_Str("Ingrese cantidad de");
//			LCDsetCursor(0, 1);
//			LCD_Write_Str("capas:");
//			LCDsetCursor(0, 2);
//			LCD_Write_Str("Cantidad:?");
//			LCDsetCursor(1, 3);
//			LCD_Write_Str("(# para continuar)");
//			LCDsetCursor(9, 2);
//			LCDblinkOn();
//			FlagUnaVez = TRUE;
//		}
//
//		if (keypadRead(&keypad,
//				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == FALSE) {
//			FlancoDebounce = TRUE;
//			cantidad_capas = KeypadToInt(tecla);
//			if (conv_table[tecla] == '#') {
//				FlagUnaVez = FALSE;
//				digito = 0;
//				MenuConfigState = MENU_VELOCIDAD;
//			}
//			if (digito < MAX_DIGITOS_EN_VALORES && conv_table[tecla] != '#')
//				LCD_Write_Char(conv_table[tecla]);
//		}
//		if (!keypadRead(&keypad,
//				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == TRUE) {
//			FlancoDebounce = FALSE;
//		}
//	}
//		break;
	case MENU_VELOCIDAD: {
		if (FlagUnaVez == FALSE) {
			LCDclear();
			LCDsetCursor(0, 0);
			LCD_Write_Str("Ingrese velocidad ");
			LCDsetCursor(0, 1);
			LCD_Write_Str("inicial (10-100%):");
			LCDsetCursor(0, 2);
			LCD_Write_Str("Velocidad:?");
			LCDsetCursor(1, 3);
			LCD_Write_Str("(# para continuar)");
			LCDsetCursor(10, 2);
			LCDblinkOn();
			FlagUnaVez = TRUE;
		}

		if (keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == FALSE) {
			FlancoDebounce = TRUE;
			velocidad = KeypadToInt(tecla);
			if (conv_table[tecla] == '#') {
				FlagUnaVez = FALSE;
				digito = 0;
				MenuConfigState = MENU_HOME_SET_PRIMERA_ESPIRA;
			}
			if (digito < MAX_DIGITOS_EN_VALORES && conv_table[tecla] != '#')
				LCD_Write_Char(conv_table[tecla]);
		}
		if (!keypadRead(&keypad,
				&tecla) && delayRead(&Delay100ms) && FlancoDebounce == TRUE) {
			FlancoDebounce = FALSE;
		}
	}
		break;
	case MENU_HOME_SET_PRIMERA_ESPIRA: {
		if (FlagUnaVez == FALSE) {
			LCDclear();
			LCDcursorOff();
			LCDblinkOff();
			LCDsetCursor(0, 0);
			LCD_Write_Str("Utilice los pulsadores");
			LCDsetCursor(0, 1);
			LCD_Write_Str("Flecha arriba y");
			LCDsetCursor(0, 2);
			LCD_Write_Str("abajo p/fijar HOME");
			LCDsetCursor(0, 3);
			LCD_Write_Str("(# para continuar)");
//			LCDsetCursor(10, 2);
			FlagUnaVez = TRUE;
		}
		/**
		 * Posicionamiento del HOME de primera espira con el motor stepper
		 * Se activan leds como indicadores de estado
		 */

		if (!gpioRead(HOME_DER) && delayRead(&Delay1) && Flanco1 == FALSE) {
			gpioWrite(LED1, ON);
			Flanco1 = TRUE;
		}
		if (Flanco1 == TRUE)
			ControlMotorPAP(100, 0);

		if (gpioRead(HOME_DER) && delayRead(&Delay1) && Flanco1 == TRUE) {
			gpioWrite(LED1, OFF);
			Flanco1 = FALSE;
		}

		if (!gpioRead(HOME_IZQ) && delayRead(&Delay2) && Flanco2 == FALSE) {
			gpioWrite(LED2, ON);
			Flanco2 = TRUE;
		}
		if (Flanco2 == TRUE)
			ControlMotorPAP(100, 1);

		if (gpioRead(HOME_IZQ) && delayRead(&Delay2) && Flanco2 == TRUE) {
			gpioWrite(LED2, OFF);
			Flanco2 = FALSE;
		}

		if (keypadRead(&keypad, &tecla) && delayRead(&Delay100ms)) {
			if (conv_table[tecla] == '#') {
				FlagUnaVez = FALSE;
				digito = 0;
				BobinadorState = CONTROL_VUELTAS;
			}
		}
	}
	}
}
/**
 * Controla los motores durante la secuencia de bobinado
 * Controla la cantidad de avance,vueltas,capas, etc
 */

void IniciaBobinado(void) {
	static uint16_t tecla = 0;
	static bool_t FlagSoloUnMensaje = TRUE;
	static bool_t Flanco3 = FALSE;
	static bool_t Flanco4 = FALSE;
	static uint8_t pruebapwm = 60;
	static uint32_t pruebavueltas = 0;
	static bool_t SentidoAvance = DERECHA;
	static uint32_t CuentaVueltas = 0;

	if (FlagSoloUnMensaje == TRUE) {
		LCDcursorOff();
		LCDblinkOff();
		LCDclear();
		LCDsetCursor(0, 0);
		LCD_Write_Str("Presione START para");
		LCDsetCursor(0, 1);
		LCD_Write_Str("iniciar el bobinado");
		FlagSoloUnMensaje = FALSE;
	}
	if (!gpioRead(START) && delayRead(&Delay1) && Flanco3 == FALSE) {
		gpioWrite(LED3, ON);
		Flanco3 = TRUE;
	}
	/**
	 * Controlo la cantidad de vueltas y el avance de alambre, si se vuelve falso
	 * alguna paro el bobinado y asumo se termino: hago otro?
	 */
	if (Flanco3 == TRUE && pruebavueltas < MAX_PULSOS_AVANCE) {
		if (CuentaVueltas >= cantidad_espiras) {
			tecla = 0;
			FlagSoloUnMensaje = TRUE;
			Flanco3 = FALSE;
			Flanco4 = FALSE;
			pruebapwm = 60;
			pruebavueltas = 0;
			SentidoAvance = DERECHA;
			CuentaVueltas = 0;
			BobinadorState = REPETIR_BOBINADO;
		}
		SoftStartMotorBobinador();
		//ControlMotorBobinador(155);
		if (CuentaVueltas > 2) {
			ControlMotorPAP(200, SentidoAvance);
			pruebavueltas++;
		}
		/**
		 * Si llegue al maximo numero de pulsos en una direccion
		 * reinicio y cambio el sentido
		 */
	} else if (Flanco3 == TRUE) {
		pruebavueltas = 0;
		if (SentidoAvance == DERECHA)
			SentidoAvance = IZQUIERDA;
		else
			SentidoAvance = DERECHA;
	}
	/**
	 * Lectura del sensor -hall
	 */

	if (Flanco3 == TRUE && LecturaSensor() == FALSE && Flanco4 == FALSE) {
		gpioWrite(LED3, ON);
		CuentaVueltas++;
		Flanco4 = TRUE;
		consolePrintUInt(CuentaVueltas);
		consolePrintEnter();

	}

	if (Flanco3 == TRUE && LecturaSensor() == TRUE && Flanco4 == TRUE) {
		Flanco4 = FALSE;
		gpioWrite(LED3, OFF);
	}
	/**
	 * Parar la secuencia de bobinado
	 */
	if (!gpioRead(STOP) && delayRead(&Delay1) && Flanco3 == TRUE) {
		gpioWrite(LED3, OFF);
		Flanco3 = FALSE;
	}
	if (Flanco3 == FALSE) {
		ControlMotorBobinador(0);
		pruebapwm = 50;
	}
	if (keypadRead(&keypad, &tecla) && delayRead(&Delay100ms)) {
		if (conv_table[tecla] == '*') {
			//FlagUnaVez = FALSE;
			//digito = 0;
			gpioWrite(LED3, OFF);
			Flanco3 = FALSE;
			SentidoAvance = DERECHA;
			FlagSoloUnMensaje == FALSE;
			BobinadorState = MENU_CONFIG_GRAL;
		}
	}
}
void RepetirBobinado(void) {
	static bool_t FlagSoloUnMensaje = TRUE;
	static uint16_t tecla = 0;
	if (FlagSoloUnMensaje == TRUE) {
		LCDclear();
		LCDsetCursor(3, 0);
		LCD_Write_Str("Repetir mismo");
		LCDsetCursor(5, 1);
		LCD_Write_Str("bobinado?");
		LCDsetCursor(4, 2);
		LCD_Write_Str("(#[Si],*[No])");
		FlagSoloUnMensaje = FALSE;
	}
	if (keypadRead(&keypad, &tecla) && delayRead(&Delay100ms)) {
		if (conv_table[tecla] == '#') {
			valorPWM = 50;
			FlagSoloUnMensaje = TRUE;
			BobinadorState = MENU_CONFIG_GRAL;
			MenuConfigState = MENU_HOME_SET_PRIMERA_ESPIRA;
		}
	}
	if (keypadRead(&keypad, &tecla) && delayRead(&Delay100ms)) {
		if (conv_table[tecla] == '*') {
			valorPWM = 50;
			FlagSoloUnMensaje = TRUE;
			BobinadorState = MENU_CONFIG_GRAL;
			MenuConfigState = MENU_DIAMETRO_ALAMBRE;
		}
	}
}
void SoftStartMotorBobinador(void) {
	//static uint8_t valorPWM = 50;

	if (delayRead(&Delay3) && valorPWM <= 229) {
		valorPWM += 10;
	}
	ControlMotorBobinador(valorPWM);
	//consolePrintUInt(valorPWM);
	//consolePrintEnter();
}
/**
 * Rutina Generica para convertir de arreglo a entero
 * @param array Arreglo a convertir
 * @param n Largo del arreglo
 * @return valor entero convertido desde el arreglo
 */

uint16_t char2int(char *array, uint8_t n) {
	int number = 0;
	int mult = 1;
	n = (int) n < 0 ? -n : n; /*cheque de valor absoluto  */
	/* para cada valor del arreglo */
	while (n--) {
		/* si noes un dito '-', verifica si el numero es > 0, break o continue */
		if ((array[n] < '0' || array[n] > '9') && array[n] != '-') {
			if (number)
				break;
			else
				continue;
		}
		if (array[n] == '-') { /* si es negativo invertir, break */
			if (number) {
				number = -number;
				break;
			}
		} else { /* convertir el digito a valor numerico  */
			number += (array[n] - '0') * mult;
			mult *= 10;
		}
	}
	return number;
}
/**
 *
 * @param teclaPresionada Desde el teclado convierte los valores presionados a char
 * @return regresa un entero equivalente hasta la introduccion de '#'
 */
uint16_t KeypadToInt(uint16_t teclaPresionada) {
	static uint8_t digitoInterno = 0;
	static uint8_t ArregloKeypadTemp[MAX_DIGITOS_EN_VALORES];
	uint16_t enteroDevuelto;
	if (digitoInterno < MAX_DIGITOS_EN_VALORES
			&& conv_table[teclaPresionada] != ','
			&& conv_table[teclaPresionada] != '#') {
		ArregloKeypadTemp[digitoInterno] = (conv_table[teclaPresionada]);
		digitoInterno++;
	} else if (conv_table[teclaPresionada] == '#') {
		enteroDevuelto = char2int(ArregloKeypadTemp, digitoInterno);
		digitoInterno = 0;
		return enteroDevuelto;
	}
}
/**
 * Funcion de control del motor paso a paso
 * @param vel_paso Periodo entre pulsos de control
 * @param dir_giro Direccion de giro
 */

void ControlMotorPAP(uint32_t vel_paso, bool_t dir_giro) {

	static uint8_t led = OFF;

	if (dir_giro == TRUE) {
		gpioWrite(GPIO0, ON);
		gpioWrite(LEDR, ON);
		gpioWrite(LEDB, OFF);
	} else if (dir_giro == FALSE) {
		gpioWrite(GPIO0, OFF);
		gpioWrite(LEDR, OFF);
		gpioWrite(LEDB, ON);
	}
	delayInaccurateUs(vel_paso);
	gpioToggle(GPIO1);

}
void ControlMotorBobinador(uint8_t Duty) {
	uint8_t valor;
	/* Usar PWM */
	valor = pwmWrite(PWM10, Duty);
}
void pwmInit(void) {
	/* Configurar PWM */
	uint8_t valor;
	valor = pwmConfig(0, PWM_ENABLE);

	valor = pwmConfig(PWM10, PWM_ENABLE_OUTPUT);

}
/**
 * Leo el sensor Hall (analogico) por el A/D filtro
 * y devuelvo booleano de nivel/flanco
 * @return Flanco devuelve el estado del flanco del sensor
 */
bool_t LecturaSensor(void) {
	static uint8_t NumMuestras = 20;
	static uint32_t Muestra = 0;
	static bool_t Flanco = TRUE;
	if (NumMuestras > 1) {
		Muestra += adcRead(CH3);
		NumMuestras--;
	} else {
		Muestra = Muestra / 20;
		NumMuestras = 20;
		if (Muestra < 400 && Flanco == TRUE) {
			Flanco = FALSE;

		}
		if (Muestra > 700 && Flanco == FALSE) {
			Flanco = TRUE;

		}
	}
	return Flanco;
}
