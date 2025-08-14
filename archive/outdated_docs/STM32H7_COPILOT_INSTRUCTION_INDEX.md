# STM32H7 Copilot Instruction Index

## 🎯 **Quick Reference Guide for AI Code Generation**

This index allows Copilot to instantly locate relevant STM32H7 documentation files without scanning the entire 78MB directory.

## 📋 **How to Use This Index**

### **1. Peripheral Lookup**
When user asks about a specific peripheral, reference these key files:

#### **GPIO (General Purpose Input/Output)**
- **Primary Files**: 198 files indexed
- **Key Documentation**:
  - `group__GPIO__Exported__Functions__Group1.md` - Initialization functions
  - `group__GPIO__Exported__Functions__Group2.md` - IO operation functions  
  - `structGPIO__InitTypeDef.md` - Configuration structure
  - `group__GPIO__mode__define.md` - Pin mode constants
  - `group__GPIO__pins__define.md` - Pin definitions
  - `group__GPIO__speed__define.md` - Speed settings

#### **SPI (Serial Peripheral Interface)**
- **Primary Files**: 199 files indexed
- **Key Documentation**:
  - `group__SPI__Exported__Functions__Group1.md` - Init/DeInit functions
  - `group__SPI__Exported__Functions__Group2.md` - IO operation functions
  - `structSPI__InitTypeDef.md` - Configuration structure
  - `group__SPI__Mode.md` - Master/Slave modes
  - `group__SPI__Data__Size.md` - Data frame formats
  - `group__SPI__BaudRate__Prescaler.md` - Clock settings

#### **UART (Universal Asynchronous Receiver Transmitter)**  
- **Primary Files**: 161 files indexed
- **Key Documentation**:
  - `group__UART__Exported__Functions__Group1.md` - Init/DeInit functions
  - `group__UART__Exported__Functions__Group2.md` - IO operation functions
  - `structUART__InitTypeDef.md` - Configuration structure
  - `group__UART__Word__Length.md` - Data bits settings
  - `group__UART__Stop__Bits.md` - Stop bit configuration
  - `group__UART__Parity.md` - Parity settings

#### **I2C (Inter-Integrated Circuit)**
- **Primary Files**: 127 files indexed  
- **Key Documentation**:
  - `group__I2C__Exported__Functions__Group1.md` - Init/DeInit functions
  - `group__I2C__Exported__Functions__Group2.md` - IO operation functions
  - `structI2C__InitTypeDef.md` - Configuration structure
  - `group__I2C__addressing__mode.md` - 7-bit/10-bit addressing
  - `group__I2C__dual__addressing__mode.md` - Dual address mode

#### **ADC (Analog to Digital Converter)**
- **Primary Files**: 89 files indexed
- **Key Documentation**:
  - `group__ADC__Exported__Functions__Group1.md` - Init/DeInit functions  
  - `group__ADC__Exported__Functions__Group2.md` - IO operation functions
  - `structADC__InitTypeDef.md` - Configuration structure
  - `group__ADC__resolution.md` - Resolution settings
  - `group__ADC__External__trigger__Source__Regular.md` - Trigger sources

#### **TIM (Timer/Counter)**
- **Primary Files**: 156 files indexed
- **Key Documentation**:
  - `group__TIM__Exported__Functions__Group1.md` - Time Base functions
  - `group__TIM__Exported__Functions__Group2.md` - Output Compare functions
  - `group__TIM__Exported__Functions__Group3.md` - Input Capture functions
  - `structTIM__Base__InitTypeDef.md` - Base timer configuration
  - `structTIM__OC__InitTypeDef.md` - Output Compare configuration

## 🔍 **Search Strategies**

### **Function Lookup Pattern**
```
Query: "HAL_GPIO_Init"
→ Reference: group__GPIO__Exported__Functions__Group1.md
→ Structure: structGPIO__InitTypeDef.md
→ Constants: group__GPIO__mode__define.md
```

### **Use Case Lookup Pattern**  
```
Query: "Configure SPI master communication"
→ Files: 
  - group__SPI__Exported__Functions__Group1.md (initialization)
  - group__SPI__Mode.md (master mode constants)
  - group__SPI__Data__Size.md (data frame size)
  - structSPI__InitTypeDef.md (configuration structure)
```

### **Peripheral Pattern Lookup**
```
Query: "UART serial communication"
→ Files:
  - group__UART__Exported__Functions__Group2.md (transmit/receive)
  - group__UART__Word__Length.md (8/9 bit data)
  - group__UART__Parity.md (none/even/odd parity)
```

## 📊 **File Categories (3,988 total files)**

- **GPIO_API**: 198 files - GPIO peripheral functions and constants
- **UART_API**: 161 files - UART/USART communication functions  
- **SPI_API**: 199 files - SPI peripheral functions and constants
- **I2C_API**: 127 files - I2C communication functions
- **ADC_API**: 89 files - Analog to Digital Converter functions
- **TIMER_API**: 156 files - Timer/Counter and PWM functions
- **DMA_API**: 87 files - Direct Memory Access functions
- **STRUCTURE_DEF**: 267 files - Structure and type definitions
- **SOURCE_CODE**: 388 files - Source code documentation
- **HAL_DRIVER**: 724 files - HAL driver documentation

## 🚀 **Common Usage Examples**

### **GPIO Configuration**
```
User Query: "Configure PA5 as output pin"
Relevant Files:
- group__GPIO__Exported__Functions__Group1.md (HAL_GPIO_Init)
- structGPIO__InitTypeDef.md (GPIO_InitTypeDef structure)  
- group__GPIO__mode__define.md (GPIO_MODE_OUTPUT_PP)
- group__GPIO__pins__define.md (GPIO_PIN_5)
```

### **UART Communication Setup**
```
User Query: "Setup UART with 9600 baud rate"
Relevant Files:
- group__UART__Exported__Functions__Group1.md (HAL_UART_Init)
- structUART__InitTypeDef.md (UART_InitTypeDef structure)
- group__UART__Word__Length.md (UART_WORDLENGTH_8B)
- group__UART__Parity.md (UART_PARITY_NONE)
```

### **SPI Master Configuration**
```
User Query: "Configure SPI1 as master for LCD communication"  
Relevant Files:
- group__SPI__Exported__Functions__Group1.md (HAL_SPI_Init)
- structSPI__InitTypeDef.md (SPI_InitTypeDef structure)
- group__SPI__Mode.md (SPI_MODE_MASTER)
- group__SPI__Data__Size.md (SPI_DATASIZE_8BIT)
```

## 💡 **Optimization Tips for Copilot**

1. **Start with Function Groups**: Always reference `group__[PERIPHERAL]__Exported__Functions__Group1.md` first for initialization
2. **Include Structure Definitions**: Add `struct[PERIPHERAL]__InitTypeDef.md` for configuration details
3. **Add Constants**: Include relevant `group__[PERIPHERAL]__[CONSTANT_TYPE]__define.md` files
4. **Context Size Management**: Each group file is typically 5-20KB (perfect for AI context)
5. **Cross-Reference Pattern**: Use related files together for complete API understanding

## 📈 **Index Statistics**

- **Total Files Indexed**: 3,988 markdown files
- **Unique Keywords**: 31,772 searchable terms
- **Function References**: 12,704 HAL/LL functions
- **Peripheral Coverage**: 11 major peripherals
- **File Categories**: 15 logical groupings

---

**Bottom Line**: This index transforms the 78MB documentation into an instantly searchable, Copilot-optimized reference system. Instead of scanning thousands of files, Copilot can directly access the 3-5 most relevant files for any STM32H7 programming task. 🎯
