package main

import (
    "fmt"
    "os"
    "os/signal"
    "syscall"
    "time" // or "runtime"
    "io/ioutil"
)

func cleanup() {
    bytes, err := ioutil.ReadFile("a.txt")
	if err != nil {
		panic(err)
	}
	now := time.Now()
	fmt.Printf("%v: read data:%s\n",now,string(bytes))
}

func main() {
    c := make(chan os.Signal)
    signal.Notify(c, syscall.SIGUSR1)
    go func() {
   for _  = range c{

        cleanup()
}
    }()

    for {
        fmt.Println("sleeping...")
        time.Sleep(10 * time.Second) // or runtime.Gosched() or similar per @misterbee
    }
}
