Welcome to Energex

# Introduction #

This project Energex hosts a software and hardware for a Twike battery management system (BMS). Due to the Twikes DC-Link voltage of about 300V...400V a lot of cells have to be connected in series. The Energex BMS is able to charge up to 192 cells connected in series. Due to its open design it can be easily adapted to arbitrary cell type: So far NiCd and LiCoMn have been tested, LiFePo will follow.


# Details #

The Energex project is divided in several sub projetcs:
  * [QTwikeAnalyzer](QTwikeAnalyzer.md), a C++ project developed with Qt is used to debug the communication between BMS and Twike converter.
  * **QTwikeSimulator**, also a C++ project is used to simulate charge/discharge and testing balancing algorithms.
  * [Supervisor](Supervisor.md) is a piece of hardware connected and powered directly from the battery stack. It measures the cell voltage and cell temperature and is able to drain any overcurrent from the cell.
  * [Mediator](Mediator.md) acts as communication interface between Supervisors and Twike converter. It is responsible to adjust charge current and voltage to the state of the battery stack.

# Forum #
Technical discussion is possible on [Energex Twike Forum](https://groups.google.com/forum/?fromgroups=#!forum/energex-twike).