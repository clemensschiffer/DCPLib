
#ifndef SLAVE_H_
#define SLAVE_H_

#include <dcp/helper/Helper.hpp>
#include <dcp/log/OstreamLog.hpp>
#include <dcp/logic/DcpManagerSlave.hpp>
#include <dcp/model/pdu/DcpPduFactory.hpp>
#include <dcp/driver/ethernet/udp/UdpDriver.hpp>

#include <cstdint>
#include <cstdio>
#include <stdarg.h>
#include <thread>
#include <cmath>


class Slave {
public:
    Slave() : stdLog(std::cout) {
        udpDriver = new UdpDriver(HOST, PORT);
		std::cout<< "GetSlaveDesciption: " << std::endl;
		SlaveDescription_t m_SlaveDescription = getSlaveDescription();
		std::cout<< "Done. Create Manager:" << std::endl;
		manager = new DcpManagerSlave(m_SlaveDescription, udpDriver->getDcpDriver());
		std::cout<< "Done" << std::endl;
        manager->setInitializeCallback<SYNC>(
                std::bind(&Slave::initialize, this));
        manager->setConfigureCallback<SYNC>(
                std::bind(&Slave::configure, this));
        manager->setSynchronizingStepCallback<SYNC>(
                std::bind(&Slave::doStep, this, std::placeholders::_1));
        manager->setSynchronizedStepCallback<SYNC>(
                std::bind(&Slave::doStep, this, std::placeholders::_1));
        manager->setRunningStepCallback<SYNC>(
                std::bind(&Slave::doStep, this, std::placeholders::_1));
        manager->setTimeResListener<SYNC>(std::bind(&Slave::setTimeRes, this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));

        //Display log messages on console
        manager->addLogListener(
                std::bind(&OstreamLog::logOstream, stdLog, std::placeholders::_1));
        manager->setGenerateLogString(true);

    }

    ~Slave() {
        delete manager;
        delete udpDriver;
    }


    void configure() {
        simulationTime = 0;
        currentStep = 0;

        //a = manager->getInput<float64_t *>(a_vr);
        a = manager->getInput<std::string *>(a_vr);
        //y = manager->getOutput<float64_t *>(y_vr);
        y = manager->getOutput<std::string *>(y_vr);
    }

    void initialize() {
        //*y = std::sin(currentStep + *a);
    }

    void doStep(uint64_t steps) {
        float64_t timeDiff =
                ((double) numerator) / ((double) denominator) * ((double) steps);


        //*y = std::sin(currentStep + *a);
		std::string tmp = "foo";
		//*y = tmp;
		std::cout << "Value of output " << *y << std::endl;
		std::cout << "Value of input"   << *a << std::endl;

		//tmp = *a;
		//*y = tmp + "a";

        //manager->Log(SIM_LOG, simulationTime, currentStep, *a, *y);
        simulationTime += timeDiff;
        currentStep += steps;
    }

    void setTimeRes(const uint32_t numerator, const uint32_t denominator) {
        this->numerator = numerator;
        this->denominator = denominator;
    }

    void start() { manager->start(); }

    SlaveDescription_t getSlaveDescription(){
        SlaveDescription_t slaveDescription = make_SlaveDescription(1, 0, "dcpslave", "b5279485-720d-4542-9f29-bee4d9a75ef9");
        slaveDescription.OpMode.SoftRealTime = make_SoftRealTime_ptr();
        Resolution_t resolution = make_Resolution();
        resolution.numerator = 1;
        resolution.denominator = 100;
        slaveDescription.TimeRes.resolutions.push_back(resolution);
        slaveDescription.TransportProtocols.UDP_IPv4 = make_UDP_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->Control =
                make_Control_ptr(HOST, 8080);
        ;
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output = make_DAT_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output->availablePortRanges.push_back(
                make_AvailablePortRange(2048, 65535));
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter = make_DAT_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter->availablePortRanges.push_back(
                make_AvailablePortRange(2048, 65535));
        slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
        slaveDescription.CapabilityFlags.canHandleReset = true;
        slaveDescription.CapabilityFlags.canHandleVariableSteps = true;
        slaveDescription.CapabilityFlags.canMonitorHeartbeat = false;
        slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
        slaveDescription.CapabilityFlags.canProvideLogOnRequest = true;
        slaveDescription.CapabilityFlags.canProvideLogOnNotification = true;

        //std::shared_ptr<Output_t> caus_y = make_Output_ptr<float64_t>();
        std::shared_ptr<Output_t> caus_y = make_Output_String_ptr();
		caus_y->String->maxSize = std::make_shared<uint32_t>(2000);
		caus_y->String->start = std::make_shared<std::string>("output_start");
        slaveDescription.Variables.push_back(make_Variable_output("y", y_vr, caus_y));
        //std::shared_ptr<CommonCausality_t> caus_a =
        //        make_CommonCausality_ptr<float64_t>();
        //caus_a->Float64->start = std::make_shared<std::vector<float64_t>>();
        //caus_a->Float64->start->push_back(10.0);

        std::shared_ptr<CommonCausality_t> caus_a =
                make_CommonCausality_String_ptr();
		caus_a->String->maxSize = std::make_shared<uint32_t>(2000);
        caus_a->String->start = std::make_shared<std::string>("input_start");
		// ?
        //caus_a->String->start = std::make_shared<std::vector<std::string>>();
		//std::string tmp = "input_start" 
        //caus_a->String->start->push_back(tmp);

        slaveDescription.Variables.push_back(make_Variable_input("a", a_vr, caus_a));
        slaveDescription.Log = make_Log_ptr();
        slaveDescription.Log->categories.push_back(make_Category(1, "DCP_SLAVE"));
        slaveDescription.Log->templates.push_back(make_Template(
                1, 1, (uint8_t) DcpLogLevel::LVL_INFORMATION, "[Time = %float64]: sin(%uint64 + %float64) = %float64"));

       return slaveDescription;
    }

private:
    DcpManagerSlave *manager;
    OstreamLog stdLog;

    UdpDriver* udpDriver;
    const char *const HOST = "127.0.0.1";
    const int PORT = 8080;

    uint32_t numerator;
    uint32_t denominator;

    double simulationTime;
    uint64_t currentStep;

    const LogTemplate SIM_LOG = LogTemplate(
            1, 1, DcpLogLevel::LVL_INFORMATION,
            "[Time = %float64]: sin(%uint64 + %float64) = %float64",
            {DcpDataType::float64, DcpDataType::uint64, DcpDataType::float64, DcpDataType::float64});

    //float64_t *a;
	std::string *a;
    const uint32_t a_vr = 2;
    //float64_t *y;
	std::string *y;
    const uint32_t y_vr = 1;

};

#endif /* SLAVE_H_ */
