#ifndef H_Persistence
#define H_Persistence

Integer Lookup NumberOfTotalBoots;
Boolean Lookup IsOK(...);
Real    Lookup TimeOfBoot(...);
Real    Lookup TimeOfLastSave(...);
Boolean Lookup CheckpointState(...);
Real    Lookup CheckpointTime(...);
String  Lookup CheckpointInfo(...);

Integer Lookup NumberOfAccessibleBoots;
Integer Lookup NumberOfUnhandledBoots;
Boolean Lookup DidCrash;
Integer Lookup CheckpointWhen(...);


Command SetCheckpoint(...);
Boolean Command SetOK(...);
Boolean Command Flush();

#endif
