import React, { Component } from 'react'
import Web3 from 'web3'
import './App.css'

class App extends Component {
  componentWillMount() {
    let parent = this;
    this.loadBlockchainData(parent);
  }

  async loadBlockchainData(parent) {
    // const web3 = new Web3(Web3.givenProvider || "http://localhost:8545")
    // const accounts = await web3.eth.getAccounts()
    // this.setState({ account: accounts[0] })

    console.log(window.ethereum);
    if (typeof window.ethereum !== 'undefined') {
      // Use the browser injected Ethereum provider
      let web3=new Web3(window.ethereum);
      //parent.setState({web3: web3});
      // Request access to the user's MetaMask account
      window.ethereum.enable();
      // Get the user's accounts
      web3.eth.getAccounts().then(function (accounts) {
          // Show the first account
          console.log('something');
          parent.setState({ account: accounts[0]});

          // 1. call API to fetch challenge james.lug.org.cn
      fetch(`${parent.serverAddr}:80/api/getChallenge`)
      // .then(handleErrors)
      .then(response => {
        console.log(response.statusText);
        return response.text();
      })
      .then(async function (data) {
          parent.setState({ statusText: 'Challenge: ' + data + 'Wait for signature...'});
          const challenge = data;
          if (challenge.length == 0) return 
          // 2. sign challenge to generate response
          parent.setState({ statusText: 'Challenge: ' + challenge + ',Verifying credentials ...'}); 
          const sig=await web3.eth.personal.sign(challenge,parent.state.account, '');
          // 3. open door
          let contractAddress = '0x37EE5C1fCf940be4e79C735F21D4E2f650f63a85';
          //fetch(`${parent.serverAddr}:80/api/checkSignature?openTime=120&amp;sig=${sig}`)
          fetch(`${parent.serverAddr}:80/api/checkSignature?sig=${sig}`)
            .then(function (response) {
              if (!response.ok) {
                parent.setState({ statusText: 'Challenge: ' + challenge + ',error:'+response.statusText}); 
                throw Error(response.statusText);
              }
              else
              {
                parent.setState({ statusText: 'Challenge: ' + challenge + ', success!'});
                return response.text()
              }
            })
            .then(function (response) {
                if (response == "pass") {
                  parent.setState({ statusText: 'Challenge: ' + challenge + 'Entrance granted!'}); 
                } else {
                  parent.setState({ statusText: 'Challenge: ' + challenge + 'Failed with: ' + response}); 
                }
            }).catch(function() {
              console.log("error blah");
            });
        //   web3.eth.personal.sign(challenge,parent.state.account, '' , function (error, value) {
        //     if (error != null) {
        //       parent.setState({ statusText: 'Challenge: ' + challenge + ',error:'+error}); 
        //     }
        //     else
        //     {
        //       parent.setState({ statusText: 'Challenge: ' + challenge + ',Verifying credentials ...'}); 
        //       // 3. open door
        //       let contractAddress = '0x37EE5C1fCf940be4e79C735F21D4E2f650f63a85';
        //       fetch(`${parent.serverAddr}:80/api/checkSignature?openTime=120&amp;sig=${value}`)
        //         .then(function (response) {
        //           if (!response.ok) {
        //             parent.setState({ statusText: 'Challenge: ' + challenge + ',error:'+response.statusText}); 
        //             throw Error(response.statusText);
        //           }
        //           else
        //           {
        //             parent.setState({ statusText: 'Challenge: ' + challenge + ', success!'});
        //             return response.text()
        //           }
        //         })
        //         .then(function (response) {
        //             if (response == "pass") {
        //               parent.setState({ statusText: 'Challenge: ' + challenge + 'Entrance granted!'}); 
        //             } else {
        //               parent.setState({ statusText: 'Challenge: ' + challenge + 'Failed with: ' + response}); 
        //             }
        //         }).catch(function() {
        //           console.log("error blah");
        //         });
        //     }
        // });
          
        })
      });
      

    } else {
        // If web3 is not available, give instructions to install MetaMask
        console.log("error");
        //parent.setState({ account: 'Please install MetaMask to connect with the Ethereum network'});
    }
  }

  constructor(props) {
    super(props)
    this.state = { web3: '', account: '', statusText: '' }
    this.serverAddr = "http://192.168.3.23";
  }

  render() {
    return (
      <div className="container">
        <h1>Hello, World!</h1>
        <p>Your account: {this.state.account}</p>
        <p>Status: {this.state.statusText}</p>
      </div>
    );
  }
}

export default App;