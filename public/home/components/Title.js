class MyElement extends HTMLElement {

	constructor() {
		super();
	}
	
	render() {
		this.shadowRoot.innerHTML = `
			<h1>
				${this.getAttribute("color")}
			</h1>
		`
	//   this.innerHTML = `
	//   <h1>Counter:  ${this.getAttribute("counter")}</h1>`;
	}
	
	connectedCallback() {
	  // browser calls this method when the element is added to the document
	  // (can be called many times if an element is repeatedly added/removed)
	  this.attachShadow({mode: 'open'});
		// console.log(this.shadowRoot.host);
	  if (!this.rendered) {
		  this.render();
		  this.rendered = true;
	  }
  
	//   setInterval(() => {
	// 	  this.setAttribute("color", +this.getAttribute("color") + 1);
	//   }, 1000)
	}

	disconnectedCallback() {
	// browser calls this method when the element is removed from the document
	// (can be called many times if an element is repeatedly added/removed)
	}

	static get observedAttributes() {
		return [/* array of attribute names to monitor for changes */"color"];
	}

	attributeChangedCallback(name, oldValue, newValue) {
	// called when one of attributes listed above is modified
	if (this.rendered)
		this.render();
	//   this.render();
	}
	
	  // there can be other element methods and properties
}

customElements.define("my-element", MyElement);