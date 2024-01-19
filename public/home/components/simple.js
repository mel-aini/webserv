import React from "./React.js";

const myReact = new React();

class ReactElement extends React {
	constructor() {
		super();
	}
}

class Component extends ReactElement {
	constructor() {
		super();
		const [num, setNum] = myReact.useState(0);
		this.num = num;
		this.setNum = setNum;
	
		this.render();
	}

	getContent() {
		return this.innerHtml;
	}

	render() {
		return this.htmlTree;
	}
}

const app = document.getElementById("app");
const C = new Component();
app.innerHTML = C.getContent();
// const c = new Component();






// setInterval(() => {
// 	setNum(num.value + 1);
// 	// console.log(num.value);
// }, 1000)
