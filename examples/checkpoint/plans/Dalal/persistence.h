#ifndef H_Persistence
#define H_Persistence

Integer Lookup NumberOfTotalBoots;
Boolean Lookup IsBootOK(...);
Real    Lookup TimeOfBoot(...);
Real    Lookup TimeOfLastSave(...);
Boolean Lookup CheckpointState(...);
Real    Lookup CheckpointTime(...);
String  Lookup CheckpointInfo(...);

Integer Lookup NumberOfAccessibleBoots;
Integer Lookup NumberOfUnhandledBoots;
Boolean Lookup DidCrash;
Integer Lookup CheckpointWhen(...);


Command set_checkpoint(...);
Boolean Command set_boot_ok(...);
Boolean Command flush_checkpoints();

#endif
