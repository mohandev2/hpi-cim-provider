include ../setting.cmpi

MOFS=$(shell ls -1 *.mof)
CLASSMOFS=$(filter-out %Register.mof, $(MOFS))
REGISTRATIONMOFS=$(filter %Register.mof, $(MOFS))

.PHONY: all clean install uninstall

all: $(MOFS)

clean: 

install:
	$(foreach MOF, $(CLASSMOFS), install -m 644 $(MOF) $(CIMOMMOF); )
	$(foreach MOF, $(REGISTRATIONMOFS), install -m 644 $(MOF) $(CIMOMMOF); )
ifdef PEGASUS
	$(foreach MOF, $(CLASSMOFS), cimmofl -uc -I. --namespace=root/cimv2 $(MOF); )
	$(foreach MOF, $(REGISTRATIONMOFS), cimmofl -uc -I. --namespace=root/PG_InterOp $(MOF); )
endif

uninstall:
	$(foreach MOF, $(MOFS), $(RM) $(CIMOMMOF)/$(MOF); )

