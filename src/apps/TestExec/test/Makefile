
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

DEBUG_CFG	:= .TestDebug.cfg

EXEC_PROG	:= ../TestExec -d $(DEBUG_CFG)
REGRESSION_PL	:= regression.pl

# Tests which run to success and require no script
EMPTY_SCRIPT		:= scripts/empty-script.plx

EMPTY_SCRIPT_TESTS	:= array2 array5 array6 array9 \
 concat1 contention1 contention3 \
 double-invariant-assignment \
 empty1 empty2 empty3 empty4 empty-string1 \
 failure-type1 failure-type2 failure-type3 failure-type4 \
 grandparent-access-plan \
 iteration-ended1 invariant1 isKnown1 \
 repeat1 repeat3 repeat4 \
 simple-assignment skip1 skip2 \
 test-abs-sqrt-plan test-node-name-scope-hack \
 test-node-name-scope test-repeat-condition \
 variables1 \
 whitespace1 \
 uninitialized-assignment

# Tests which run to success and require a script of the same name
SAME_NAME_SCRIPT_TESTS	:= array1 array3 array4 array8 atomic-assignment \
 boolean1 \
 change-lookup-test command1 command2 command3 command4 concat2 conjuncts conjuncts1 \
 function-call1 \
 lookup1 lookup2 lookup3 \
 repeat2 repeat5 \
 simple-functioncall site-survey \
 test-end-condition test-timepoint \
 update-lookup-test update-test

# Resource arbitration tests, which require comparing the output with a known standard
RESOURCE_ARBITRATION_TESTS	:= resource1 resource1-repeatCond \
 resource2-equalPriority \
 resource3-ackRel resource3-denyHP resource3-deny2HP \
 resource4-hvm resource4-hvm-repeatCond \
 resource-renewable1 \
 NonUnaryResources

# Simple-drive tests
SIMPLE_DRIVE_PLAN	:= simple-drive
SIMPLE_DRIVE_SCRIPTS	:= single-drive double-drive

exec-regression-tests: assign-parent-invariant-test empty-script-tests same-name-script-tests
exec-regression-tests: simple-drive-tests command-handles-tests resource-arbitration-tests

assign-parent-invariant-test:
	@echo Running assign-to-parent-invariant test
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	tst=assign-to-parent-invariant ; \
	$(EXEC_PROG) -p plans/$${tst}.plx -s $(EMPTY_SCRIPT) > $${tst}.out ; \
	perl $(REGRESSION_PL) $${tst}.out valid/RUN_$${tst}_empty-script.valid

empty-script-tests:
	@echo Running tests with the empty script
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	for tst in $(EMPTY_SCRIPT_TESTS) ; do \
		echo ' ' $${tst} ; \
		$(EXEC_PROG) -p plans/$${tst}.plx -s $(EMPTY_SCRIPT) > $${tst}.out ; \
		perl $(REGRESSION_PL) $${tst}.out ; \
	done

same-name-script-tests:
	@echo Running tests with their own scripts
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	for tst in $(SAME_NAME_SCRIPT_TESTS) ; do \
		echo ' ' $${tst} ; \
		$(EXEC_PROG) -p plans/$${tst}.plx -s scripts/$${tst}-script.plx > $${tst}.out ; \
		perl $(REGRESSION_PL) $${tst}.out ; \
	done

simple-drive-tests:
	@echo Running simple-drive tests
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	for scrpt in $(SIMPLE_DRIVE_SCRIPTS) ; do \
		echo ' ' $${scrpt} ; \
		$(EXEC_PROG) -p plans/$(SIMPLE_DRIVE_PLAN).plx -s scripts/$${scrpt}-script.plx > $${scrpt}.out ; \
		perl $(REGRESSION_PL) $${scrpt}.out ; \
	done

command-handles-tests:
	@echo Running command handles tests
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	for openclosed in open closed ; do \
		for plan in a1 a3 ; do \
			planname=simple-$${openclosed}loop-command-$${plan} ; \
			for script in a1 a3 ; do \
				scriptname=simple-$${openclosed}loop-command-$${script}-script ; \
				logname=$${planname}_$${script}.out ; \
				echo ' ' $${planname} $${scriptname} ; \
				$(EXEC_PROG) -p plans/$${planname}.plx -s scripts/$${scriptname}.plx > $${logname} ; \
				perl $(REGRESSION_PL) $${logname} valid/RUN_$${planname}_$${scriptname}.valid ; \
			done ; \
		done ; \
	done
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	planname=simple-openloop-command-nopost ; \
	scriptname=simple-openloop-command-a3-script ; \
	echo ' ' $${planname} ; \
	$(EXEC_PROG) -p plans/$${planname}.plx -s scripts/$${scriptname}.plx > $${planname}.out ; \
	perl $(REGRESSION_PL) $${planname}.out valid/RUN_$${planname}_$${scriptname}.valid
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	planname=simple-closedloop-command-multipleAck ; \
	scriptname=$${planname}-script ; \
	echo ' ' $${planname} ; \
	$(EXEC_PROG) -p plans/$${planname}.plx -s scripts/$${scriptname}.plx > $${planname}.out ; \
	perl $(REGRESSION_PL) $${planname}.out valid/RUN_$${planname}_$${scriptname}.valid

resource-arbitration-tests:
	@echo Running resource arbitration tests
	@. $(PLEXIL_HOME)/bin/plexil-setup.sh && \
	for tst in $(RESOURCE_ARBITRATION_TESTS) ; do \
		echo ' ' $${tst} ; \
		$(EXEC_PROG) -p plans/$${tst}.plx -s scripts/$${tst}-script.plx > $${tst}.out ; \
		perl $(REGRESSION_PL) $${tst}.out valid/RUN_$${tst}_$${tst}-script.valid ; \
	done

localclean::
	-$(RM) *.out RegressionResults

# Targets that should be made every time they are invoked
.PHONY: assign-parent-invariant-test empty-script-tests same-name-script-tests

.PHONY: simple-drive-tests command-handles-tests resource-arbitration-tests

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
